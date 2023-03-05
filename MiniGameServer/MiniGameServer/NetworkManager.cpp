#include "pch.h"

#include "DataBaseManager.h"
#include "NetworkManager.h"
#include "OverlappedManager.h"
#include "UserManager.h"
#include "RoomManager.h"
#include "RoomUnit.h"
#include "PlayerUnit.h"
#include "Log.h"

NetworkManager::NetworkManager( ) : m_listenSocket( INVALID_SOCKET ), m_iocpHandle( NULL )
{
}

NetworkManager::~NetworkManager( ) noexcept = default;

bool NetworkManager::Initialize( )
{
	// 초기 세팅 진행
	WSADATA wsaData;
	int returnValue = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( returnValue != 0 )
	{
		PRINT_LOG( "WSAStartup failed" );
		return false;
	}

	m_listenSocket = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );

	if ( m_listenSocket == INVALID_SOCKET )
	{
		PRINT_LOG( "WSASocket Initialize() failed" );
		return false;
	}

	// Nagle Off Option
	int socketOption = 1;
	returnValue = setsockopt( m_listenSocket, SOL_SOCKET, TCP_NODELAY, reinterpret_cast< const char* >( &socketOption ), sizeof( socketOption ) );
	if ( returnValue != 0 )
	{
		PRINT_LOG( "setsockopt Initialize() failed" );
		return false;
	}

	std::cout << "Server Initialize Sueccess..." << std::endl;
	return true;
}

bool NetworkManager::Listen( )
{
	// 서버 bind & listen 진행
	SOCKADDR_IN serverAddr;
	ZeroMemory( &serverAddr, sizeof( serverAddr ) );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( InitServer::SERVERPORT );
	serverAddr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );

	int returnValue = bind( m_listenSocket, reinterpret_cast< sockaddr* >( &serverAddr ), sizeof( serverAddr ) );
	if ( returnValue != 0 )
	{
		PRINT_LOG( "bind failed" );
		return false;
	}

	returnValue = listen( m_listenSocket, SOMAXCONN );
	if ( returnValue != 0 )
	{
		PRINT_LOG( "listen failed" );
		return false;
	}

	std::cout << "Waiting For Player..." << std::endl;

	return true;
}

bool NetworkManager::RunServer( )
{
	// IOCP 객체 생성 
	m_iocpHandle = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 0 );
	if ( m_iocpHandle == NULL )
	{
		PRINT_LOG( "CreateIoCompletionPort Failed m_iocpHandle Init" );
		return false;
	}
	if ( CreateIoCompletionPort( reinterpret_cast< HANDLE >( m_listenSocket ), m_iocpHandle, 0, 0 ) == NULL )
	{
		PRINT_LOG( "CreateIoCompletionPort Failed Connect Process" );
		return false;
	}
	
	WSAOVERLAPPED_EXTEND over;
	Accept( &over );

	// 메인 워크 프로세스 연결, accept, receive, send 
	std::vector< std::thread > workerThreads;
	for ( int i = 0; i < ( InitServer::TOTALCORE / 2 ); ++i )
	{
		workerThreads.emplace_back( [ & ]( ) { MainWorkProcess( ); } );
	}
	
	// DataBaseManager::GetInstance( );
	UserManager::GetInstance().Run();
	RoomManager::GetInstance().Run();
	RoomManager::GetInstance().RunTimer();

	for ( auto& wthread : workerThreads )
	{
		wthread.join( );
	}

	closesocket( m_listenSocket );
	WSACleanup( );

	return true;
}

void NetworkManager::ReassemblePacket( char* packet, const DWORD& bytes, const SOCKET& socket )
{
	if ( packet == nullptr || bytes == 0 )
	{
		PRINT_LOG( "packet == nullptr" );
		return;
	}
	// 각 유저별 패킷 재조립

	// 해딩 패킷 사이즈 만큼 왔는지 검사
	// 패킷의 사이즈만큼 다 왔다면 다음 netbuffer read 위치 초기화
	// 패킷의 사이즈만큼 다 안왔다면, Read 위치 갱신 및 패킷 타입에 따른 처리

	UserManager::GetInstance( ).PushTask(
		[ socket, packet, bytes ]( )
	{
		auto& users = UserManager::GetInstance( ).GetUsers( );
		int startReceive = users[ socket ]->GetPreviousReceivePosition( );
		int byte = bytes;

		while(byte > 0 )
		{
			unsigned char packetSize = packet[ 0 ];

			if ( packetSize > byte  )
			{
				const int previousPosition = startReceive + byte;
				users[ socket ]->SetPreviousReceivePosition( previousPosition );
				break;
			}
			else
			{
				char completePacket[ InitPacket::MAX_PACKETSIZE ];
				std::copy( packet, packet + packetSize, completePacket );

				// Process Packet
				UserManager::GetInstance().ProcessPacket( socket, completePacket );
				byte -= packetSize;

				// Init Receive Buffer
				char initBuffer[ InitPacket::MAX_BUFFERSIZE ];

				std::copy( packet + packetSize, packet + InitPacket::MAX_BUFFERSIZE, initBuffer );
				std::copy( initBuffer, initBuffer + InitPacket::MAX_BUFFERSIZE, packet );

				users[ socket ]->SetPreviousReceivePosition( 0 );
			}
		}
		users[ socket ]->ReceivePacket( );
	} );
	
}

void NetworkManager::Disconnect( const SOCKET& socket )
{
	//방에서 종료한 플레이어 삭제
	RoomManager::GetInstance().PushTask(
		[ socket ]()
		{
			auto& users = UserManager::GetInstance().GetUsers();
			
			if ( users.find( socket ) == users.end() )
				return;
			if ( !users[ socket ] )
				return;

			int id = users[ socket ]->GetId();
			int roomNum = users[ socket ]->GetRoomNum();

			RoomManager::GetInstance().GetRooms()[ roomNum ].PopPlayer( socket );
			if ( RoomManager::GetInstance().GetRooms()[ roomNum ].GetPlayers().empty() )
			{
				RoomManager::GetInstance().GetRooms().erase( roomNum );
				RoomManager::GetInstance().PushRoomNumber( roomNum );
				std::cout << "방 삭제" << std::endl;
			}
			else
			{
				//같은 방 플레이어들에게 종료한 플레이어 알림
			}
			//접속 종료 플레이어 유저 관리 객체에서 삭제
			UserManager::GetInstance().PushTask(
				[ socket ]()
				{
					// 같은 방 사람들에게 플레이어 종료 정보 전송 

					//유저객체 유저풀에 전달
					auto& users = UserManager::GetInstance().GetUsers();
					std::cout << users[ socket ]->GetId() << " 유저 접속 종료" << std::endl;

					UserManager::GetInstance().PushPlayerUnit( users[ socket ] );
					UserManager::GetInstance().PushPlayerId( users[ socket ]->GetId() );
					users.erase( socket );
				} );
		} );
}

bool NetworkManager::Accept( WSAOVERLAPPED_EXTEND* over )
{
	SOCKET socket = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if ( socket == INVALID_SOCKET )
	{
		PRINT_LOG( "WSASocket failed" );
		return false;
	}

	int socketOption = 1;
	int returnValue = setsockopt( socket, SOL_SOCKET, TCP_NODELAY, reinterpret_cast< const char* >( &socketOption ), sizeof( socketOption ) );
	if ( returnValue != 0 )
	{
		PRINT_LOG( "setsockopt Accept()" );
		return false;
	}

	ZeroMemory( &over->over, sizeof( over->over ) );

	DWORD bytes;
	over->type = EOperationType::ACCEPT;
	over->socket = socket;

	bool returnValue2 = AcceptEx( m_listenSocket, socket, over->networkBuffer, 0, sizeof( SOCKADDR_IN ) + 16, sizeof( SOCKADDR_IN ) + 16, &bytes, &over->over );
	if ( returnValue2 == false )
	{
		//작업이 성공적으로 시작, 아직 진행 중
		if ( WSAGetLastError( ) == ERROR_IO_PENDING )
			return true;

		PRINT_LOG( "AcceptEx Accept()" );
		return false;
	}

	return true;
}

void NetworkManager::MainWorkProcess( )
{
	while ( true )
	{
		DWORD bytes;
		ULONG_PTR completionKey;
		WSAOVERLAPPED* over;

		bool returnValue = GetQueuedCompletionStatus( m_iocpHandle, &bytes, &completionKey, &over, INFINITE );
		SOCKET userKey = static_cast< SOCKET > ( completionKey );
		WSAOVERLAPPED_EXTEND* overExtend = reinterpret_cast< WSAOVERLAPPED_EXTEND* >( over );

		switch ( overExtend->type )
		{
		case EOperationType::RECV:
		{
			if ( bytes == 0 || returnValue == false )
			{
				Disconnect( userKey );
				break;
			}
			
			ReassemblePacket( overExtend->networkBuffer, bytes, userKey );
		}
		break;

		case EOperationType::SEND:
		{
			if ( bytes == 0 || returnValue == false )
			{
				Disconnect( userKey );
				break;
			}
			OverlappedManager::GetInstance( ).PushOverlapped( overExtend );
		}
		break;

		case EOperationType::ACCEPT:
		{
			if ( returnValue == false )
			{
				Disconnect( userKey );
				break;
			}

			userKey = overExtend->socket;
			std::cout << "ACCEPT Player [" << userKey << "]" << std::endl;

			// Player 생성
			UserManager::GetInstance( ).PushTask(
				[ userKey ]( )
				{
					auto& users = UserManager::GetInstance( ).GetUsers( );
					const int userCount = static_cast< int >( users.size( ) );
					if ( userCount <= InitServer::MAX_PLAYERNUM )
					{
						users[ userKey ] = UserManager::GetInstance().GetPlayerUnit();
						users[ userKey ]->SetSocket( userKey );
						users[ userKey ]->SetOverlappedOperation( EOperationType::RECV );
						users[ userKey ]->SetState( EClientState::ACCESS ); 
						users[ userKey ]->SetId( UserManager::GetInstance().GetPlayerId() );

						Packet::FirstPlayer packet( users[ userKey ]->GetId() );

						users[ userKey ]->SendPacket( packet );

						HANDLE returnValue2 = CreateIoCompletionPort( reinterpret_cast< HANDLE >( userKey ), NetworkManager::GetInstance().GetIocpHandle(), userKey, 0 );
						if ( returnValue2 == NULL )
						{
							PRINT_LOG( "CreateIoCompletionPort AddNewClient()" );
							return;
						}

						users[ userKey ]->ReceivePacket( );
					}
				} );

			// Prototype -> 플레이어 접속 후 3명 존재 시 바로 게임으로 넘어가도록
			// 방으로 이동
			RoomManager::GetInstance( ).PushTask(
				[ userKey ]( )
				{
					int roomNum = -1;
					auto& rooms = RoomManager::GetInstance( ).GetRooms( );
					for ( auto& [roomIndex, roomUnit] : rooms )
					{
						if ( roomUnit.GetPlayers( ).size( ) == 3 )
							continue;
						roomNum = roomIndex;
						break;
					}
					// 방 새로 생성 - 3명 이하인 방이 없을 시 
					if ( roomNum == -1 )
					{
						roomNum = RoomManager::GetInstance( ).GetNewRoomNumber( );
					}
 					auto& currentRoom = rooms[ roomNum ];
					
					// 방에 접속 유저 추가
					currentRoom.PushPlayer( userKey );

					// 유저 상태 갱신 및 현재 접속한 방 입력
					UserManager::GetInstance( ).PushTask(
						[ userKey, roomNum ]( )
						{
							UserManager::GetInstance( ).GetUsers( )[ userKey ]->SetState( EClientState::MATCHING );
							UserManager::GetInstance( ).GetUsers( )[ userKey ]->SetRoomNumber( roomNum );
						} );

					// 현재 방에 3명 존재 시 게임 시작
					if ( currentRoom.GetPlayers( ).size( ) == 3 )
					{
						std::cout << "방에 3명 입장" << std::endl;
						// 방에 있는 플레이어들에게 각각의 플레이어들 초기 정보 전송 (고유 색, 이름 등)
						const std::vector<SOCKET> others = currentRoom.GetPlayers();
						UserManager::GetInstance().PushTask(
							[ userKey, others ]()
							{
								auto& user = UserManager::GetInstance().GetUsers();
								if ( user.find( userKey ) == user.end() )
								{
									PRINT_LOG( "user == nullptr" );
									return;
								}
								int count = 1;
								for ( const auto& player : others )
								{
									count = 1;
									for ( const auto& other : others )
									{
										user[ player ]->SetState( EClientState::GAME );
										// 게임 시작 요청 클라이언트에게 보내기
										Packet::GameStart packet( user[ other ]->GetId() );
										packet.color = count;
										packet.x = InitPlayer::INITPOSITION_X + InitPlayer::INITINTERVAL * ( count - 1 );
										user[ player ]->SendPacket( packet );
										std::cout << player <<"에게 " <<other<<"정보 "<< "게임 시작 패킷 전송" << std::endl;
										++count;
									}
								}
							} );
					}
				} );

			Accept( overExtend );
		}
		break;
		}
	}

}
