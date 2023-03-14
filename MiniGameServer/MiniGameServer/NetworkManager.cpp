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
	const char socketOption = 1;
	returnValue = setsockopt( m_listenSocket, SOL_SOCKET, TCP_NODELAY,  &socketOption, sizeof( socketOption ) );
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

#if NDEBUG
	DataBaseManager::GetInstance().DBConnect();
#endif
	UserManager::GetInstance().Run();
	RoomManager::GetInstance().Run();

	for ( auto& wthread : workerThreads )
	{
		wthread.join( );
	}

	closesocket( m_listenSocket );
	WSACleanup( );

	return true;
}

void NetworkManager::ReassemblePacket( char* packet, const DWORD bytes, const SOCKET socket )
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
		PlayerUnit* user = UserManager::GetInstance( ).GetUser( socket );
		if ( !user )
			return;
		const int startReceive = user->GetPreviousReceivePosition( );
		int byte = bytes;

		while(byte > 0 )
		{
			unsigned char packetSize = packet[ 0 ];

			if ( packetSize > byte  )
			{
				const int previousPosition = startReceive + byte;
				user->SetPreviousReceivePosition( previousPosition );
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

				user->SetPreviousReceivePosition( 0 );
			}
		}
		user->ReceivePacket( );
	} );
	
}

void NetworkManager::Disconnect( const SOCKET socket )
{
	//방에서 종료한 플레이어 삭제
	RoomManager::GetInstance().PushTask(
		[ socket ]()
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser(socket);		

			if ( !user )
			{
				PRINT_LOG( "존재하지 않는 유저입니다." );
				return;
			}

			const int id = user->GetId();
			const int roomNum = user->GetRoomNum();
			const EClientState state = user->GetClientState();

			//접속 종료 플레이어 유저 관리 객체에서 삭제
			UserManager::GetInstance().PushTask(
			[ socket ]()
			{
				// 같은 방 사람들에게 플레이어 종료 정보 전송 

				//유저객체 유저풀에 전달
				PlayerUnit* user = UserManager::GetInstance().GetUser( socket );
				if ( !user )
				{
					PRINT_LOG( "user == nullptr" );
					return;
				}
				std::cout << user->GetSocket() << " 유저 접속 종료" << std::endl;

				UserManager::GetInstance().PushPlayerUnit( user );
				UserManager::GetInstance().PushPlayerId( user->GetId() );
				UserManager::GetInstance().DeleteUser( socket );
			} );
			
			//매칭 중이지 않거나, 게임 중 혹은 게임 종료 한 플레이어의 경우 방 객체에 불필요하게 접근하지 않습니다.
			if ( state < EClientState::MATCHING || state >  EClientState::GAMEFINISH )
			{
				return;
			}

			RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
			if ( !room )
			{
				PRINT_LOG( "room == nullptr" );
				return;
			}

			room->PopPlayer( socket );
			if ( room->GetPlayers().empty() )
			{
				RoomManager::GetInstance().PushRoom( room );
				RoomManager::GetInstance().PushRoomNumber( roomNum );
				RoomManager::GetInstance().DeleteRoom( roomNum );
				PRINT_LOG( "방 삭제" );
			}
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

	const int socketOption = 1;
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
			UserManager::GetInstance().PushTask(
				[ userKey ]()
				{
					auto& users = UserManager::GetInstance().GetUsers();
					const int userCount = static_cast< int >( users.size( ) );
					if ( userCount <= InitServer::MAX_PLAYERNUM )
					{
						users[ userKey ] = UserManager::GetInstance().GetPlayerUnitFromPools();
						users[ userKey ]->SetSocket( userKey );
						users[ userKey ]->SetOverlappedOperation( EOperationType::RECV );
						users[ userKey ]->SetClientState( EClientState::ACCESS ); 
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

			Accept( overExtend );
		}
		break;
		}
	}

}
