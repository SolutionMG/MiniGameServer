#include "pch.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "RoomUnit.h"
#include "Log.h"
#include "DataBaseManager.h"
#include "RoomManager.h"
#include "MathManager.h"

UserManager::UserManager( )
	: BaseTaskManager( )
	, m_users ( )
{
	for ( int i = 0; i < InitServer::MAX_PLAYERNUM; ++i )
	{
		PlayerUnit* player = new PlayerUnit(INVALID_SOCKET);
		player->Initialize();
		m_userPools.push( player );
	}

	for ( int i = 0; i < InitServer::MAX_PLAYERNUM; ++i )
	{
		m_pIdPools.push( i );
	}

	AddProcess();
}

UserManager::~UserManager( )
{
	for ( auto& [id, user] : m_users )
	{
		delete user;
		user = nullptr;
	}
	m_users.clear( );

	PlayerUnit* player = nullptr;
	while ( m_userPools.try_pop( player ) )
	{
		delete player;
		player = nullptr;
	}
}

void UserManager::ProcessPacket( const SOCKET& socket, char* packet )
{
	if ( !packet )
	{
		PRINT_LOG( "packet == nullptr" );
		return;
	}

	if ( m_users.find( socket ) == m_users.end( ) )
	{
		PRINT_LOG( "socket == null, �������� �ʴ� �����κ����� ��Ŷ�Դϴ�." );
		return;
	}

	if ( m_processFunctions.find( packet[ 1 ] ) == m_processFunctions.end() )
	{
		PRINT_LOG( "���� ��Ŷ Ÿ���Դϴ�." );
		return;
	}

	m_processFunctions[ packet[ 1 ] ]( socket, packet );
}

void UserManager::AddProcess( )
{
	m_processFunctions.reserve( 10 );
	m_processFunctions.emplace( std::make_pair( ClientToServer::LOGIN_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void { return ProcessLoginRequest( socket, packet ); } ) ));
	m_processFunctions.emplace( std::make_pair( ClientToServer::MOVE, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void { return ProcessMove( socket, packet ); } ) ) );
	m_processFunctions.emplace( std::make_pair( ClientToServer::SKILLUSE_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void { return ProcessSkill( socket, packet ); } ) ) );
}

void UserManager::ProcessLoginRequest( const SOCKET& socket, char* packet )
{
	if ( !packet )
	{
		PRINT_LOG( "packet == nullptr" );
		return;
	}

	if ( m_users.find( socket ) == m_users.end() )
	{
		PRINT_LOG( "socket == null, �������� �ʴ� �����κ����� ��Ŷ�Դϴ�." );
		return;
	}

	// �α��� ��û �� 
	PlayerUnit* player = m_users[ socket ];
	int id = player->GetId();

	Packet::LoginRequest data = *reinterpret_cast< Packet::LoginRequest* > ( packet );
	int baseScore = 0;

#if NDEBUG
	if ( DataBaseManager::GetInstance().LogOn( data.name, data.password, baseScore ) )
	{
		Packet::LoginResult send( id, ServerToClient::LOGON_OK );
		player->SetName( data.name );
		player->SendPacket( send );
	}
	else
	{
		Packet::LoginResult send( id, ServerToClient::LOGON_FAILED );
		player->SendPacket( send );
	}
#endif // _DEBUG

}

void UserManager::ProcessSignupRequest( const SOCKET& socket, char* packet )
{
	//Packet::sign data = *reinterpret_cast< Packet::LoginRequest* > ( packet );

}

void UserManager::ProcessMove( const SOCKET& socket, char* packet )
{
	Packet::Move send = *reinterpret_cast< Packet::Move* > ( packet );
	send.info.type = ServerToClient::MOVE;

	if ( m_users.find( socket ) == m_users.end() )
	{
		PRINT_LOG( "�������� �ʴ� ���� �Դϴ�." );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		PRINT_LOG( "�������� �ʴ� ���� �Դϴ�." );
		return;
	}

	//����
	Position previousPos = player->GetPosition();
	Position currentPos = Position( send.x, send.y );

	float distance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, currentPos.x, currentPos.y );
	float predictDistance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, previousPos.x * InitServer::MAX_DISTANCE, previousPos.y * InitServer::MAX_DISTANCE );

	if ( distance > predictDistance )
	{
		PRINT_LOG( "�̻��� ��ǥ�� ����" );
		return;
	}

	player->SetPosition( currentPos );
	const short color = player->GetColor();

	int roomNum = player->GetRoomNum();
	RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );

	if ( !room )
	{
		PRINT_LOG( "�������� �ʴ� ���Դϴ�." );
		return;
	}
	auto& players = room->GetPlayers();
	// �̵�
	{
		// ���� �濡 �ִ� �÷��̾�鿡�� ���� ����
		for ( const auto& index : players )
		{
			if ( index == socket )
				continue;

			m_users[ index ]->SendPacket( send );
		}
	}

	// �� �浹
	{
		//unsigned char returnValue = MathManager::GetInstance().CheckCollisionWall( currentPos.x, currentPos.y );
		//if ( returnValue != InitWorld::NOTWALLCOLLISION)
		//{
		//	Packet::CollisionWall wall( player->GetId(), returnValue );
		//	wall.directionX = send.directionX;
		//	wall.directionY = send.directionY;
		//	for ( const auto& index : players )
		//	{
		//		m_users[ index ]->SendPacket( wall );
		//	}
		//}
	}

	// �÷��̾� �� �浹
	{
		Packet::CollisionPlayer cp;
		cp.owners[ 0 ] = player->GetId();
		int count = 1;
		bool collision = false;
		for ( const auto& index : players )
		{
			if ( index == socket )
				continue;

			if ( m_users.find( index ) == m_users.end() )
				continue;

			//�浹 �˻�
			const Position temp = m_users[ index ]->GetPosition();
			//std::cout << player->GetColor() << "���� �÷��̾�� " << m_users[ index ]->GetColor() << "���� �÷��̾� �浹 �˻�" << std::endl;
			if ( MathManager::GetInstance().CollisionSphere( currentPos.x, currentPos.y, temp.x, temp.y ) )
			{
				collision = true;

				if ( count < 1 || count > 2 )
					continue;

				cp.owners[ count++ ] = m_users[ index ]->GetId();

				if ( m_users[ index ]->GetStronger() )
				{
					m_users[ index ]->SetStronger( false );
					m_users[ index ]->SetSkillDuration( 0 );
				}

				// 1 ���� 2 �Ķ� 3 ���
				// std::cout << "!!!!!!!!" << player->GetColor() << "���� �÷��̾�� " << m_users[ index ]->GetColor() << "���� �÷��̾� �浹 �߻�!!!!!!!!" << std::endl;

			}
		}

		// ���ΰ� �ٸ� �÷��̾� �� �浹 ��� ����
		if ( collision )
		{
			if ( player->GetStronger() )
			{
				player->SetStronger( false );
				player->SetSkillDuration( 0 );
			}
			for ( const auto& index : players )
			{
				m_users[ index ]->SendPacket( cp );
			}
		}
	}
	
	// ���� �浹
	{
		int xIndex = static_cast< int >( ( currentPos.x - ( InitWorld::FIRST_TILEPOSITION_X - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //Ÿ���� X�ε���
		int yIndex = static_cast< int >( ( currentPos.y - ( InitWorld::FIRST_TILEPOSITION_Y - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //Ÿ���� Y�ε���
		int blockIndex = xIndex + ( yIndex * InitWorld::TILE_COUNTX );

		if ( blockIndex < 0 || blockIndex > 48 )
		{
			PRINT_LOG( "�� �ܺ��� ��ǥ ����" );
			return;
		}

		Tile tile = room->GetTile( blockIndex );

		if ( tile.color == color )
			return;
		
		if ( MathManager::GetInstance().CollisionPointAndRectangle( currentPos.x, currentPos.y, tile.x, tile.y) )
		{
			//std::cout << player->GetId() << "�� �浹 ���� ����" << blockIndex << std::endl;
			//Ÿ�� �� �浹 �÷��̾� ������ ����
			RoomManager::GetInstance().PushTask(
				[ blockIndex, roomNum, color, socket ]()
				{
					RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
					if ( !room )
					{
						PRINT_LOG( "�������� �ʴ� ���Դϴ�." );
						return;
					}
					auto& players = room->GetPlayers();
					short tilecolor = room->GetTile( blockIndex ).color;

					UserManager::GetInstance().PushTask(
						[ socket, players, blockIndex, tilecolor ]()
						{
							unsigned char basePlayerScore = 0;
							int basePlayer = -1;
							PlayerUnit* player = UserManager::GetInstance().GetUser( socket );

							//�Ͼ� �� �����̸� ������ �϶���ų �÷��̾ ����
							if ( tilecolor != 0 )
							{
								// ������ �ƴ� �ش� ������ ���� ���� �÷��̾� ���� �϶�
								for ( auto& index : players )
								{
									if ( index == socket )
										continue;

									PlayerUnit* user = UserManager::GetInstance().GetUser( index );

									if ( user->GetColor() == tilecolor )
									{
										// �ش� �÷��̾� ������ �������̶�� 1�� �̻��̾�� ��.
										basePlayerScore = user->GetScore();

										if ( basePlayerScore == 0 )
										{
											break;
										}

										basePlayer = user->GetId();
										user->SetScore( --basePlayerScore );
										break;
									}
								}
							}

							// mp���� ����

							unsigned char mp = player->GetMp();
							if ( mp != InitPlayer::SKILLENABLE && !player->GetStronger())
							{
								mp = min( mp + InitPlayer::MPCOUNT, InitPlayer::SKILLENABLE );
								player->SetMp( mp );

								Packet::PlayerMp_Update mpPacket( player->GetId(), mp );
								player->SendPacket( mpPacket );
							}

							// ���� �浹 �÷��̾� ���� ���
							unsigned char newPlayerScore = player->GetScore() + 1;
							player->SetScore( newPlayerScore );

							Packet::Score upScore( player->GetId(), newPlayerScore );
							Packet::Score downScore( basePlayer, basePlayerScore );

							// ���� �濡 �ִ� �÷��̾�鿡�� ���� ����
							for ( const auto& index : players )
							{
								// �浹 ���� ����
								Packet::CollisionTile collisionPacket = Packet::CollisionTile( player->GetId(), blockIndex);
								UserManager::GetInstance().GetUser( index )->SendPacket( collisionPacket );

								//����
								//// ���� ��� ���� ����
								//m_users[ index ]->SendPacket( upScore );

								//// ���� �϶���ų �÷��̾ ���� ��� �ش� ���� ����
								//if ( basePlayer != -1 )
								//{
								//	m_users[ index ]->SendPacket( downScore );
								//}

							}
						} );
					room->SetTileColor( blockIndex, color );
				} );
		}
	}
}

void UserManager::ProcessSkill( const SOCKET& socket, char* packet )
{
	Packet::SkillUse_Request send = *reinterpret_cast< Packet::SkillUse_Request* > ( packet );

	if ( m_users.find( socket ) == m_users.end() )
	{
		PRINT_LOG( "�������� �ʴ� ���� �Դϴ�." );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		PRINT_LOG( "�������� �ʴ� ���� �Դϴ�." );
		return;
	}
	Packet::SkillUse_Result result(player->GetId(), ServerToClient::SKILLUSE_REQUEST_FAILED);

	//PRINT_LOG( "��ų ��� ��û ��Ŷ �۽�" );
	if ( player->GetMp() == InitPlayer::SKILLENABLE )
	{
		RoomUnit* room = RoomManager::GetInstance().GetRoom(player->GetRoomNum());
		if ( !room )
		{
			PRINT_LOG( "�������� �ʴ� �� �Դϴ�." );
			return;
		}
		
		result.info.type = ServerToClient::SKILLUSE_REQUEST_SUCCESS;

		player->SetMp(0);
		player->SetStronger( true );

		for (const auto& player : room->GetPlayers() )
		{
			m_users[ player ]->SendPacket( result );
		}

		//PRINT_LOG( "��ų ��� ���� ��Ŷ ����" );

		return;
	}
	
	//��� �Ұ� ��Ŷ
	player->SendPacket( result );
	//PRINT_LOG( "��ų ��� ���� ��Ŷ ����" );
}

void UserManager::DeleteUser( const SOCKET& socket )
{
	m_users.erase( socket );
}

void UserManager::PushPlayerUnit( PlayerUnit* player )
{
	if ( !player )
		return;

	player->Initialize();
	m_userPools.push( player );
}

void UserManager::PushPlayerId( const int& id )
{
	if ( id < 0 )
		return;
	m_pIdPools.push( id );
}

PlayerUnit* UserManager::GetUser( const SOCKET& socket )
{
	if ( m_users.find( socket ) == m_users.end() )
		return nullptr;

	return  m_users[ socket ]; 
}

PlayerUnit* UserManager::GetPlayerUnitFromPools( )
{
	PlayerUnit* player = nullptr;
	if ( !m_userPools.try_pop( player ) )
	{
		player = new PlayerUnit(INVALID_SOCKET);
		player->Initialize();
	}

	return player;
}

const int UserManager::GetPlayerId()
{
	// TODO: ���⿡ return ���� �����մϴ�.
	int id = -1;
	if ( !m_pIdPools.try_pop( id ) )
	{
		id = static_cast< int >( m_pIdPools.unsafe_size() );
		++id;
	}
	return id;
}
