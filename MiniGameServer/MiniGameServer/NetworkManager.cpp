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
	// �ʱ� ���� ����
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
	// ���� bind & listen ����
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
	// IOCP ��ü ���� 
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

	// ���� ��ũ ���μ��� ����, accept, receive, send 
	std::vector< std::thread > workerThreads;
	for ( int i = 0; i < ( InitServer::TOTALCORE / 2 ); ++i )
	{
		workerThreads.emplace_back( [ & ]( ) { MainWorkProcess( ); } );
	}
	
	// DataBaseManager::GetInstance( );
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

void NetworkManager::ReassemblePacket( char* packet, const DWORD& bytes, const SOCKET& socket )
{
	if ( packet == nullptr || bytes == 0 )
	{
		PRINT_LOG( "packet == nullptr" );
		return;
	}
	// �� ������ ��Ŷ ������
	UserManager::GetInstance( ).PushTask(
		[ socket, packet, bytes ]( )
	{
		auto& users = UserManager::GetInstance( ).GetUsers( );
		int startReceive = users[ socket ]->GetPreviousReceivePosition( );
		int byte = bytes;
		unsigned short packetSize = packet[ 0 ];

		// �ص� ��Ŷ ������ ��ŭ �Դ��� �˻�
		// ��Ŷ�� �����ŭ �� �Դٸ� ���� netbuffer read ��ġ �ʱ�ȭ
		// ��Ŷ�� �����ŭ �� �ȿԴٸ�, Read ��ġ ���� �� ��Ŷ Ÿ�Կ� ���� ó��

		if ( packetSize > byte + startReceive )
		{
			const int previousPosition = startReceive + byte;
			users[ socket ]->SetPreviousReceivePosition( previousPosition );
		}
		else
		{
			char completePacket[ InitPacket::MAX_PACKETSIZE ];
			memcpy_s( completePacket, sizeof( completePacket ), packet, packetSize );
			memcpy_s( packet, sizeof( InitPacket::MAX_BUFFERSIZE ), packet + packetSize, ( byte + startReceive - packetSize ) );

			///Process Packet
			UserManager::GetInstance( ).ProcessPacket( socket, completePacket );
			users[ socket ]->SetPreviousReceivePosition( static_cast< unsigned char >( byte + startReceive - packetSize ) );
		}                             

		users[ socket ]->ReceivePacket( );
	} );
	
}

void NetworkManager::Disconnect( const SOCKET& socket )
{
	//�濡�� ������ �÷��̾� ����
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
				std::cout << "�� ����" << std::endl;
			}
			else
			{
				//���� �� �÷��̾�鿡�� ������ �÷��̾� �˸�
			}
			//���� ���� �÷��̾� ���� ���� ��ü���� ����
			UserManager::GetInstance().PushTask(
				[ socket ]()
				{
					// ���� �� ����鿡�� �÷��̾� ���� ���� ���� 

					//������ü ����Ǯ�� ����
					auto& users = UserManager::GetInstance().GetUsers();
					std::cout << users[ socket ]->GetId() << " ���� ���� ����" << std::endl;

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
		//�۾��� ���������� ����, ���� ���� ��
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

			// Player ����
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

			// Prototype -> �÷��̾� ���� �� 3�� ���� �� �ٷ� �������� �Ѿ����
			// ������ �̵�
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
					// �� ���� ���� - 3�� ������ ���� ���� �� 
					if ( roomNum == -1 )
					{
						roomNum = RoomManager::GetInstance( ).GetNewRoomNumber( );
					}
 					auto& currentRoom = rooms[ roomNum ];
					
					// �濡 ���� ���� �߰�
					currentRoom.PushPlayer( userKey );

					// ���� ���� ���� �� ���� ������ �� �Է�
					UserManager::GetInstance( ).PushTask(
						[ userKey, roomNum ]( )
						{
							UserManager::GetInstance( ).GetUsers( )[ userKey ]->SetState( EClientState::MATCHING );
							UserManager::GetInstance( ).GetUsers( )[ userKey ]->SetRoomNumber( roomNum );
						} );

					// ���� �濡 3�� ���� �� ���� ����
					if ( currentRoom.GetPlayers( ).size( ) == 3 )
					{
						std::cout << "�濡 3�� ����, ���� ���� ��Ŷ ����" << std::endl;
						// �濡 �ִ� �÷��̾�鿡�� ������ �÷��̾�� �ʱ� ���� ���� (���� ��, �̸� ��)
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
								int color = 0;
								for ( const auto& player : others )
								{
									for ( const auto& other : others )
									{
										user[ other ]->SetState( EClientState::GAME );
										// ���� ���� ��û Ŭ���̾�Ʈ���� ������
										Packet::GameStart packet( user[ player ]->GetId() );
										packet.color = color;
										user[ other ]->SendPacket( packet );
									}
									++color;
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
