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
	// �� ������ ��Ŷ ������

	// �ص� ��Ŷ ������ ��ŭ �Դ��� �˻�
	// ��Ŷ�� �����ŭ �� �Դٸ� ���� netbuffer read ��ġ �ʱ�ȭ
	// ��Ŷ�� �����ŭ �� �ȿԴٸ�, Read ��ġ ���� �� ��Ŷ Ÿ�Կ� ���� ó��

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
	//�濡�� ������ �÷��̾� ����
	RoomManager::GetInstance().PushTask(
		[ socket ]()
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser(socket);		

			if ( !user )
			{
				PRINT_LOG( "�������� �ʴ� �����Դϴ�." );
				return;
			}

			const int id = user->GetId();
			const int roomNum = user->GetRoomNum();
			const EClientState state = user->GetClientState();

			//���� ���� �÷��̾� ���� ���� ��ü���� ����
			UserManager::GetInstance().PushTask(
			[ socket ]()
			{
				// ���� �� ����鿡�� �÷��̾� ���� ���� ���� 

				//������ü ����Ǯ�� ����
				PlayerUnit* user = UserManager::GetInstance().GetUser( socket );
				if ( !user )
				{
					PRINT_LOG( "user == nullptr" );
					return;
				}
				std::cout << user->GetSocket() << " ���� ���� ����" << std::endl;

				UserManager::GetInstance().PushPlayerUnit( user );
				UserManager::GetInstance().PushPlayerId( user->GetId() );
				UserManager::GetInstance().DeleteUser( socket );
			} );
			
			//��Ī ������ �ʰų�, ���� �� Ȥ�� ���� ���� �� �÷��̾��� ��� �� ��ü�� ���ʿ��ϰ� �������� �ʽ��ϴ�.
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
				PRINT_LOG( "�� ����" );
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
