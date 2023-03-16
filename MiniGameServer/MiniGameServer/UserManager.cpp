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

void UserManager::ProcessPacket( const SOCKET socket, char* packet )
{
	if ( !packet )
	{
		WARN_LOG( "packet == nullptr" );
		return;
	}

	if ( m_users.find( socket ) == m_users.end( ) )
	{
		WARN_LOG( "socket == null, �������� �ʴ� �����κ����� ��Ŷ�Դϴ�." );
		return;
	}

	if ( m_processFunctions.find( packet[ 1 ] ) == m_processFunctions.end() )
	{
		WARN_LOG( "���� ��Ŷ Ÿ���Դϴ�." );
		return;
	}

	COMMON_LOG(
		"[RECV] Socket : %d, PacketType : %s", 
		socket,
		WsyEnum::ToString( static_cast< PACKET_TYPE::CTOS >( static_cast< int >( packet[ 1 ] ) ) ).c_str() );
	
	m_processFunctions[ packet[ 1 ] ]( socket, packet );
}

void UserManager::AddProcess( )
{
	m_processFunctions.reserve( 10 );
	m_processFunctions.emplace( std::make_pair( ClientToServer::LOGIN_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void		{ return ProcessLoginRequest( socket, packet ); } ) ));
	m_processFunctions.emplace( std::make_pair( ClientToServer::SIGNUP_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void		{ return ProcessSignupRequest( socket, packet ); } ) ) );
	m_processFunctions.emplace( std::make_pair( ClientToServer::MOVE, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void				{ return ProcessMove( socket, packet ); } ) ) );
	m_processFunctions.emplace( std::make_pair( ClientToServer::SKILLUSE_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void	{ return ProcessSkill( socket, packet ); } ) ) );
	m_processFunctions.emplace( std::make_pair( ClientToServer::MATHCING_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void	{ return ProcessMatchingRequest( socket, packet ); } ) ) );
	m_processFunctions.emplace( std::make_pair( ClientToServer::QUIT_ROOM, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void			{ return ProcessQuitRoom( socket, packet ); } ) ) );
}

void UserManager::ProcessLoginRequest( const SOCKET socket, char* packet )
{
	// �α��� ��û 
	if ( !packet )
	{
		WARN_LOG( "packet == nullptr" );
		return;
	}

	Packet::LoginRequest data = *reinterpret_cast< Packet::LoginRequest* > ( packet );

	if ( m_users.find( socket ) == m_users.end() )
	{
		WARN_LOG( "socket == nullptr" );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "player == nullptr" );
		return;
	}

	const int id = player->GetId();
	int bestScore = 0;
	Packet::LoginResult send( id, ServerToClient::LOGON_FAILED );

#if NDEBUG
	// �̹� �α����� ���̵� �˻�
	for ( const auto& [ _, user ] : m_users )
	{
		std::string curName = DataBaseManager::GetInstance().DecodingString( data.name );
		if ( user->GetName() == curName )
		{
			WARN_LOG( "�ߺ� �α��� [ name : %s ]", curName.c_str() );
			send.info.type = ServerToClient::LOGIN_DUPLICATION;
			player->SendPacket( send );
			return;
		}
	}

	if ( DataBaseManager::GetInstance().LogOn( data.name, data.password, bestScore ) )
	{
		send.info.type = ServerToClient::LOGON_OK;
		send.bestScore = bestScore;
		player->SetName( DataBaseManager::GetInstance().DecodingString( data.name ) );
		player->SetBestScore( bestScore );
		player->SendPacket( send );
		player->SetClientState( EClientState::LOGON );

		INFO_LOG( "�ߺ� �α��� [ name : %s ]", player->GetName().c_str() );

		return;
	}
#endif // _DEBUG

	player->SendPacket( send );

}

void UserManager::ProcessSignupRequest( const SOCKET socket, char* packet )
{
	// ȸ������ ��û
	if ( !packet )
	{
		WARN_LOG( "packet == nullptr" );
		return;
	}

	const Packet::SignUpRequest data = *reinterpret_cast< Packet::SignUpRequest* > ( packet );

	if ( m_users.find( socket ) == m_users.end() )
	{
		WARN_LOG( "socket == nullptr" );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "player == nullptr" );
		return;
	}

	Packet::SignUpResult result( player->GetId(), ServerToClient::SIGNUP_FAILED );
#if NDEBUG
	if ( DataBaseManager::GetInstance().SignUp( data.name, data.password ) )
	{
		result.info.type = ServerToClient::SIGNUP_OK ;
		player->SetName( data.name );
		player->SendPacket( result );
		INFO_LOG( "ȸ������ ���� [ socket : %d , %s ]", socket, player->GetName().c_str() );
		return;
	}
#endif
	player->SendPacket( result );
	WARN_LOG( "ȸ������ ���� [ socket : %d , %s ]", socket, player->GetName().c_str() );
}

void UserManager::ProcessMove( const SOCKET socket, char* packet )
{
	Packet::Move send = *reinterpret_cast< Packet::Move* > ( packet );
	send.info.type = ServerToClient::MOVE;

	if ( m_users.find( socket ) == m_users.end() )
	{
		WARN_LOG( "�������� �ʴ� ���� �Դϴ�. [ socket : %d ]", socket );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "�������� �ʴ� ���� �Դϴ�. [ socket : %d ]", socket );
		return;
	}

	if ( player->GetClientState() != EClientState::GAME )
		return;

	const Position previousPos = player->GetPosition();
	const Position currentPos = Position( send.x, send.y );

	const float distance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, currentPos.x, currentPos.y );
	const float predictDistance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, previousPos.x * InitServer::MAX_DISTANCE, previousPos.y * InitServer::MAX_DISTANCE );

	if ( distance > predictDistance )
	{
		WARN_LOG( "�̻��� ��ǥ�� ���� [ distance : %d, predictDistance : %d ]", distance, predictDistance );
		return;
	}

	player->SetPosition( currentPos );
	const int roomNum = player->GetRoomNum();

	RoomManager::GetInstance().PushTask(
	[ roomNum, socket, send, currentPos ]()
	{
		RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
		if ( !room )
		{
			WARN_LOG( "�������� �ʴ� ���Դϴ� [ roomNum: %d ]", roomNum );
			return;
		}

		auto& players = room->GetPlayers();

		UserManager::GetInstance().PushTask(
		[ players, socket, send, currentPos, roomNum ]()
		{
			PlayerUnit* player = UserManager::GetInstance().GetUser( socket );
			if ( !player )
			{
				WARN_LOG( "�������� �ʴ� �����Դϴ� [ socket: %d ]", socket );
				return;
			}
			const short color = player->GetColor();
			auto& users = UserManager::GetInstance().GetUsers();
			// �̵�
			{
				// ���� �濡 �ִ� �÷��̾�鿡�� ���� ����
				for ( const auto index : players )
				{
					if ( index == socket )
						continue;
					UserManager::GetInstance().GetUser( index )->SendPacket( send );
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
				//		UserManager::GetInstance().GetUser( index )->SendPacket( wall );
				//	}
				//}
			}

			// �÷��̾� �� �浹
			{
				Packet::CollisionPlayer cp;
				Packet::StunStart stunStart;

				cp.owners[ 0 ] = player->GetId();
				int count = 1;
				bool collision = false;
				bool strongerExist = false;

				if ( player->GetPlayerState() == EPlayerState::STRONGER )
				{
					cp.strongers[ 0 ] = player->GetId();
					strongerExist = true;
				}
				else
				{
					stunStart.owners[ 0 ] = player->GetId();
				}

				for ( const auto index : players )
				{
					if ( index == socket )
						continue;

					if ( users.find( index ) == users.end() )
						continue;

					//�浹 �˻�
					const Position temp = users[ index ]->GetPosition();
					//std::cout << player->GetColor() << "���� �÷��̾�� " << m_users[ index ]->GetColor() << "���� �÷��̾� �浹 �˻�" << std::endl;
					if ( MathManager::GetInstance().CollisionSphere( currentPos.x, currentPos.y, temp.x, temp.y ) )
					{
						collision = true;

						if ( count < 1 || count > 2 )
							continue;

						cp.owners[ count ] = users[ index ]->GetId();

						if ( users[ index ]->GetPlayerState() == EPlayerState::STRONGER )
						{
							strongerExist = true;
							cp.strongers[ count ] = users[ index ]->GetId();
						}
						else if ( users[ index ]->GetPlayerState() == EPlayerState::NORMAL )
						{
							stunStart.owners[ count ] = users[ index ]->GetId();
						}

						++count;
						// 1 ���� 2 �Ķ� 3 ���
						// std::cout << "!!!!!!!!" << player->GetColor() << "���� �÷��̾�� " << m_users[ index ]->GetColor() << "���� �÷��̾� �浹 �߻�!!!!!!!!" << std::endl;

					}
				}

				// ���ΰ� �ٸ� �÷��̾� �� �浹 ��� ����
				if ( collision )
				{
					if ( player->GetPlayerState() == EPlayerState::STRONGER )
					{
						player->SetPlayerState( EPlayerState::NORMAL );
						player->SetSkillDuration( 0 );
					}

					Packet::SkillEnd end[ 3 ]{cp.strongers[0], cp.strongers[1], cp.strongers[2]};

					for ( const auto index : players )
					{
						//���� ���� ����
						if ( strongerExist )
						{
							if ( users[ index ]->GetPlayerState() == EPlayerState::STRONGER )
							{
								users[ index ]->SetPlayerState( EPlayerState::NORMAL );
								users[ index ]->SetSkillDuration( 0 );

								INFO_LOG( "���� ���� [ id : %d ] ", users[ index ]->GetSocket() );
							}
							else if ( users[ index ]->GetPlayerState() == EPlayerState::NORMAL )
							{
								users[ index ]->SetPlayerState( EPlayerState::STUN );
								users[ index ]->SetStunDuration( 0 );

								INFO_LOG( "���� �߻� [ id : %d ] ", users[ index ]->GetSocket() );
							}
							
							//��ų ���� ���� ����

							for ( int i = 0; i < 3; ++i )
							{
								if ( end[ i ].owner == -1 )
									continue;
								users[ index ]->SendPacket( end[i]);
							}
							
							users[ index ]->SendPacket( stunStart );
						}

						// �浹 ���� ����
						users[ index ]->SendPacket( cp );


					}
				}
			}

			// ���� �浹
			{
				const int xIndex = static_cast< int >( ( currentPos.x - ( InitWorld::FIRST_TILEPOSITION_X - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //Ÿ���� X�ε���
				const int yIndex = static_cast< int >( ( currentPos.y - ( InitWorld::FIRST_TILEPOSITION_Y - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //Ÿ���� Y�ε���
				const int blockIndex = xIndex + ( yIndex * InitWorld::TILE_COUNTX );

				if ( blockIndex < 0 || blockIndex > 48 )
				{
					WARN_LOG( "�� �ܺ��� ��ǥ ���� [ xIndex : %d, yIndex : %d, blockIndex : %d ]", xIndex, yIndex, blockIndex );
					return;
				}

				{
					//std::cout << player->GetId() << "�� �浹 ���� ����" << blockIndex << std::endl;
					//Ÿ�� �� �浹 �÷��̾� ������ ����
					RoomManager::GetInstance().PushTask(
					[ blockIndex, roomNum, socket, currentPos, color ]()
					{
						RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
						if ( !room )
						{
							WARN_LOG( "�������� �ʴ� ���Դϴ�. [ roomNum :  %d ]", roomNum );
							return;
						}

						const Tile tile = room->GetTile( blockIndex );
						const short beforeColor = tile.color;
						if ( beforeColor == color )
							return;

						if ( MathManager::GetInstance().CollisionPointAndRectangle( currentPos.x, currentPos.y, tile.x, tile.y ) )
						{
							// ���� �� ����
							room->SetTileColor( blockIndex, color );

							auto& players = room->GetPlayers();

							UserManager::GetInstance().PushTask(
							[ socket, players, blockIndex, beforeColor ]()
							{
								unsigned char basePlayerScore = 0;
								int basePlayer = -1;
								PlayerUnit* player = UserManager::GetInstance().GetUser( socket );

								//�Ͼ� �� �����̸� ������ �϶���ų �÷��̾ ����
								if ( beforeColor != 0 )
								{
									// ������ �ƴ� �ش� ������ ���� ���� �÷��̾� ���� �϶�
									for ( auto& index : players )
									{
										if ( index == socket )
											continue;

										PlayerUnit* user = UserManager::GetInstance().GetUser( index );

										if ( user->GetColor() == beforeColor )
										{
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
								if ( mp != InitPlayer::SKILLENABLE && player->GetPlayerState() != EPlayerState::STRONGER )
								{
									mp = min( mp + InitPlayer::MPCOUNT, InitPlayer::SKILLENABLE );
									player->SetMp( mp );

									Packet::PlayerMpUpdate mpPacket( player->GetId(), mp );
									player->SendPacket( mpPacket );
								}

								// ���� �浹 �÷��̾� ���� ���
								const unsigned char newPlayerScore = player->GetScore() + 1;

								player->SetScore( newPlayerScore );

								const Packet::Score upScore( player->GetId(), newPlayerScore );
								const Packet::Score downScore( basePlayer, basePlayerScore );

								// ���� �濡 �ִ� �÷��̾�鿡�� ���� ����
								for ( const auto index : players )
								{
									// �浹 ���� ����
									Packet::CollisionTile collisionPacket = Packet::CollisionTile( player->GetId(), blockIndex );
									UserManager::GetInstance().GetUser( index )->SendPacket( collisionPacket );

									// ���� ��� ���� ����
									UserManager::GetInstance().GetUser( index )->SendPacket( upScore );
									// ���� �϶���ų �÷��̾ ���� ��� �ش� ���� ����
									if ( basePlayer != -1 )
									{
										UserManager::GetInstance().GetUser( index )->SendPacket( downScore );
									}
								}
							} );
						}
					} );
				}
			}
		} );

	} );
	
}

void UserManager::ProcessSkill( const SOCKET socket, char* packet )
{
	//const Packet::SkillUseRequest send = *reinterpret_cast< Packet::SkillUseRequest* > ( packet );

	if ( m_users.find( socket ) == m_users.end() )
	{
		WARN_LOG( "�������� �ʴ� �����Դϴ�. [ socket :  %d ]", socket );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "�������� �ʴ� �����Դϴ�. [ socket :  %d ]", socket );
		return;
	}
	const Packet::SkillUseResult result(player->GetId(), ServerToClient::SKILLUSE_REQUEST_FAILED);

	//PRINT_LOG( "��ų ��� ��û ��Ŷ �۽�" );
	if ( player->GetMp() == InitPlayer::SKILLENABLE )
	{
		int roomNum = player->GetRoomNum();
		RoomManager::GetInstance().PushTask(
		[ roomNum, socket ]( )
		{
			RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
			if ( !room )
			{
				WARN_LOG( "�������� �ʴ� ���Դϴ�. [ roomNum :  %d ]", roomNum );
				return;
			}
			auto& players = room->GetPlayers();

			UserManager::GetInstance().PushTask(
			[ players, socket ]()
			{
				PlayerUnit* player = UserManager::GetInstance().GetUser( socket );
				if ( !player )
					return;
				const Packet::SkillUseResult result( player->GetId(), ServerToClient::SKILLUSE_REQUEST_SUCCESS );
				player->SetMp( 0 );
				player->SetPlayerState( EPlayerState::STRONGER );

				for ( const auto index : players )
				{
					UserManager::GetInstance().GetUser( index )->SendPacket(result);
				}

				INFO_LOG( "��ų ��� ���� ��Ŷ ���� [ socket : %d ]", socket );
			} );
		} );

		return;
	}
	
	//��� �Ұ� ��Ŷ
	player->SendPacket( result );
	//PRINT_LOG( "��ų ��� ���� ��Ŷ ����" );
}

void UserManager::ProcessMatchingRequest( const SOCKET socket, char* packet )
{
	//const Packet::MatchingRequest send = *reinterpret_cast< Packet::MatchingRequest* > ( packet );

	// �÷��̾� �α��� �� 3�� ���� �� �ٷ� �������� �Ѿ����
	// ������ �̵�
	RoomManager::GetInstance().PushTask(
	[ socket ]()
	{
		int roomNum = -1;
		auto& rooms = RoomManager::GetInstance().GetRooms();
		RoomUnit* currentRoom = nullptr;
		for ( auto& [roomIndex, roomUnit] : rooms )
		{
			if ( roomUnit->GetState() == RoomState::GAME )
				continue;
			roomNum = roomIndex;
			currentRoom = roomUnit;
			break;
		}

		// �� ���� ���� - 3�� ������ ���� ���� �� 
		if ( roomNum == -1 )
		{
			roomNum = RoomManager::GetInstance().GetNewRoomNumber();
			currentRoom = RoomManager::GetInstance().GetRoomUnitFromPools();
			currentRoom->SetState( RoomState::MATCHING );
			rooms[ roomNum ] = currentRoom;
		}

		if ( !currentRoom )
		{
			WARN_LOG( "currentRoom == nullptr [ roomNum : %d ]", roomNum );
			return;
		}

		// �濡 ���� ���� �߰�
		currentRoom->PushPlayer( socket );

		// ���� ���� ���� �� ���� ������ �� �Է�
		UserManager::GetInstance().PushTask(
		[ socket, roomNum ]()
		{
			UserManager::GetInstance().GetUser( socket )->SetClientState( EClientState::MATCHING );
			UserManager::GetInstance().GetUser( socket )->SetRoomNumber( roomNum );
		} );

		// ���� �濡 3�� ���� �� ���� ����
		if ( currentRoom->GetPlayers().size() == InitWorld::INGAMEPLAYER_NUM )
		{
			INFO_LOG( "�濡 3�� ���� [ roomNum : %d ]", roomNum );
			currentRoom->SetState( RoomState::GAME );

			// �濡 �ִ� �÷��̾�鿡�� ������ �÷��̾�� �ʱ� ���� ���� (���� ��, �̸� ��)
			const std::vector<SOCKET> others = currentRoom->GetPlayers();
			UserManager::GetInstance().PushTask(
			[ roomNum, socket, others ]()
			{
				auto& users = UserManager::GetInstance().GetUsers();
				if ( users.find( socket ) == users.end() )
				{
					WARN_LOG( "user == nullptr [ socket : %d ]", socket );
					return;
				}
				int count = 1;
				for ( const auto index : others )
				{
					count = 1;
					for ( const auto other : others )
					{
						// �ΰ��� �÷��̾� �ʱ�ȭ ���� Ŭ���̾�Ʈ���� ������
						Packet::InitPlayers packet( users[ other ]->GetId() );
						packet.color = count;

						const Position pos = Position( InitPlayer::INITPOSITION_X[ count - 1 ], InitPlayer::INITPOSITION_Y[ count - 1 ] );
						//��ġ �����ϱ�
						packet.x = pos.x;
						packet.y = pos.y;
						packet.directionX = InitPlayer::INITDIRECTION_X[ count - 1 ];
						packet.directionY = InitPlayer::INITDIRECTION_Y[ count - 1 ];
						strcpy_s( packet.name, DataBaseManager::GetInstance().EncodingString( users[ other ]->GetName().c_str() ).c_str() );

						users[ other ]->SetPosition( pos );
						users[ other ]->SetColor( count );
						users[ index ]->SendPacket( packet );

						//�ʱ� ���� Ÿ�� �� ����
						Packet::CollisionTile tile( packet.owner, InitWorld::FIRSTTILE_INDEX[ count - 1 ] );
						users[ index ]->SendPacket( tile );

						++count;
					}
				}

				INFO_LOG( "���ӽ��� ��Ŷ ���� �Ϸ�" );

				for ( const auto index : others )
				{
					users[ index ]->SetClientState( EClientState::GAME );

				}

				RoomManager::GetInstance().PushTask(
				[ roomNum ]()
				{
					//Ÿ�̸� ����
					RoomManager::GetInstance().PushTimer( roomNum );

					//�ʱ� ���� Ÿ�� �� ����
					auto& room = RoomManager::GetInstance().GetRooms()[ roomNum ];
					for ( int i = 0; i < 3; ++i )
						room->SetTileColor( InitWorld::FIRSTTILE_INDEX[ i ], i + 1 );
				} );

			} );
		}
	} );

}

void UserManager::ProcessQuitRoom( const SOCKET socket, char* packet )
{
	Packet::QuitRoom send = *reinterpret_cast< Packet::QuitRoom* > ( packet );

	PlayerUnit* user = UserManager::GetInstance().GetUser( socket );

	if ( !user )
	{
		WARN_LOG( "�������� �ʴ� �����Դϴ�. [ socket : %d ]", socket );
		return;
	}

	int id = user->GetId();
	int roomNum = user->GetRoomNum();
	EClientState state = user->GetClientState();

	if ( state != EClientState::GAMEFINISH )
	{
		WARN_LOG( "���� �� �� ������ ����� ������ �ƴմϴ�. [ socket : %d ]", socket );
		return;
	}

	RoomManager::GetInstance().PushTask(
	[ socket, roomNum ]()
	{
		RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
		if ( !room )
		{
			WARN_LOG( "room == nullptr [ roomNum : %d ]", roomNum );
			return;
		}

		room->PopPlayer( socket );
		if ( room->GetPlayers().empty() )
		{
			RoomManager::GetInstance().PushRoom( room );
			RoomManager::GetInstance().PushRoomNumber( roomNum );
			RoomManager::GetInstance().DeleteRoom( roomNum );
			INFO_LOG( "�濡 ������ �÷��̿� 0�� - �� ���� [ roomNum : %d ]", roomNum );
		}

		UserManager::GetInstance().PushTask(
		[ socket ]()
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser( socket );
			user->SetClientState( EClientState::LOGON );
			user->SetRoomNumber( -1 );
			user->SetScore( 1 );
			user->SetPosition( Position( 0.f, 0.f ) );
			user->SetMp( 0 );
			user->SetPlayerState( EPlayerState::NORMAL);
			user->SetSkillDuration( 0 );
			user->SetStunDuration( 0 );
		} );
	} );

	INFO_LOG( "QuitRoom ��Ŷ - �� ������ �Ϸ� [ roomNum : %d ]", roomNum );
}

void UserManager::DeleteUser( const SOCKET socket )
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

void UserManager::PushPlayerId( const int id )
{
	if ( id < 0 )
		return;
	m_pIdPools.push( id );
}

PlayerUnit* UserManager::GetUser( const SOCKET socket )
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
