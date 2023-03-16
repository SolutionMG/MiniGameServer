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
		WARN_LOG( "socket == null, 존재하지 않는 유저로부터의 패킷입니다." );
		return;
	}

	if ( m_processFunctions.find( packet[ 1 ] ) == m_processFunctions.end() )
	{
		WARN_LOG( "없는 패킷 타입입니다." );
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
	// 로그인 요청 
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
	// 이미 로그인한 아이디 검사
	for ( const auto& [ _, user ] : m_users )
	{
		std::string curName = DataBaseManager::GetInstance().DecodingString( data.name );
		if ( user->GetName() == curName )
		{
			WARN_LOG( "중복 로그인 [ name : %s ]", curName.c_str() );
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

		INFO_LOG( "중복 로그인 [ name : %s ]", player->GetName().c_str() );

		return;
	}
#endif // _DEBUG

	player->SendPacket( send );

}

void UserManager::ProcessSignupRequest( const SOCKET socket, char* packet )
{
	// 회원가입 요청
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
		INFO_LOG( "회원가입 성공 [ socket : %d , %s ]", socket, player->GetName().c_str() );
		return;
	}
#endif
	player->SendPacket( result );
	WARN_LOG( "회원가입 실패 [ socket : %d , %s ]", socket, player->GetName().c_str() );
}

void UserManager::ProcessMove( const SOCKET socket, char* packet )
{
	Packet::Move send = *reinterpret_cast< Packet::Move* > ( packet );
	send.info.type = ServerToClient::MOVE;

	if ( m_users.find( socket ) == m_users.end() )
	{
		WARN_LOG( "존재하지 않는 유저 입니다. [ socket : %d ]", socket );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "존재하지 않는 유저 입니다. [ socket : %d ]", socket );
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
		WARN_LOG( "이상한 좌표를 수신 [ distance : %d, predictDistance : %d ]", distance, predictDistance );
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
			WARN_LOG( "존재하지 않는 방입니다 [ roomNum: %d ]", roomNum );
			return;
		}

		auto& players = room->GetPlayers();

		UserManager::GetInstance().PushTask(
		[ players, socket, send, currentPos, roomNum ]()
		{
			PlayerUnit* player = UserManager::GetInstance().GetUser( socket );
			if ( !player )
			{
				WARN_LOG( "존재하지 않는 유저입니다 [ socket: %d ]", socket );
				return;
			}
			const short color = player->GetColor();
			auto& users = UserManager::GetInstance().GetUsers();
			// 이동
			{
				// 같은 방에 있는 플레이어들에게 정보 전송
				for ( const auto index : players )
				{
					if ( index == socket )
						continue;
					UserManager::GetInstance().GetUser( index )->SendPacket( send );
				}
			}

			// 벽 충돌
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

			// 플레이어 간 충돌
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

					//충돌 검사
					const Position temp = users[ index ]->GetPosition();
					//std::cout << player->GetColor() << "색상 플레이어와 " << m_users[ index ]->GetColor() << "색상 플레이어 충돌 검사" << std::endl;
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
						// 1 빨강 2 파랑 3 노랑
						// std::cout << "!!!!!!!!" << player->GetColor() << "색상 플레이어와 " << m_users[ index ]->GetColor() << "색상 플레이어 충돌 발생!!!!!!!!" << std::endl;

					}
				}

				// 본인과 다른 플레이어 간 충돌 사실 전달
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
						//스턴 정보 전송
						if ( strongerExist )
						{
							if ( users[ index ]->GetPlayerState() == EPlayerState::STRONGER )
							{
								users[ index ]->SetPlayerState( EPlayerState::NORMAL );
								users[ index ]->SetSkillDuration( 0 );

								INFO_LOG( "스턴 공격 [ id : %d ] ", users[ index ]->GetSocket() );
							}
							else if ( users[ index ]->GetPlayerState() == EPlayerState::NORMAL )
							{
								users[ index ]->SetPlayerState( EPlayerState::STUN );
								users[ index ]->SetStunDuration( 0 );

								INFO_LOG( "스턴 발생 [ id : %d ] ", users[ index ]->GetSocket() );
							}
							
							//스킬 종료 정보 전송

							for ( int i = 0; i < 3; ++i )
							{
								if ( end[ i ].owner == -1 )
									continue;
								users[ index ]->SendPacket( end[i]);
							}
							
							users[ index ]->SendPacket( stunStart );
						}

						// 충돌 정보 전송
						users[ index ]->SendPacket( cp );


					}
				}
			}

			// 발판 충돌
			{
				const int xIndex = static_cast< int >( ( currentPos.x - ( InitWorld::FIRST_TILEPOSITION_X - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //타일의 X인덱스
				const int yIndex = static_cast< int >( ( currentPos.y - ( InitWorld::FIRST_TILEPOSITION_Y - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //타일의 Y인덱스
				const int blockIndex = xIndex + ( yIndex * InitWorld::TILE_COUNTX );

				if ( blockIndex < 0 || blockIndex > 48 )
				{
					WARN_LOG( "맵 외부의 좌표 수신 [ xIndex : %d, yIndex : %d, blockIndex : %d ]", xIndex, yIndex, blockIndex );
					return;
				}

				{
					//std::cout << player->GetId() << "의 충돌 정보 전송" << blockIndex << std::endl;
					//타일 색 충돌 플레이어 색으로 변경
					RoomManager::GetInstance().PushTask(
					[ blockIndex, roomNum, socket, currentPos, color ]()
					{
						RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
						if ( !room )
						{
							WARN_LOG( "존재하지 않는 방입니다. [ roomNum :  %d ]", roomNum );
							return;
						}

						const Tile tile = room->GetTile( blockIndex );
						const short beforeColor = tile.color;
						if ( beforeColor == color )
							return;

						if ( MathManager::GetInstance().CollisionPointAndRectangle( currentPos.x, currentPos.y, tile.x, tile.y ) )
						{
							// 발판 색 변경
							room->SetTileColor( blockIndex, color );

							auto& players = room->GetPlayers();

							UserManager::GetInstance().PushTask(
							[ socket, players, blockIndex, beforeColor ]()
							{
								unsigned char basePlayerScore = 0;
								int basePlayer = -1;
								PlayerUnit* player = UserManager::GetInstance().GetUser( socket );

								//하얀 색 발판이면 점수를 하락시킬 플레이어가 없음
								if ( beforeColor != 0 )
								{
									// 본인이 아닌 해당 발판의 기존 색상 플레이어 점수 하락
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

								// mp정보 갱신
								unsigned char mp = player->GetMp();
								if ( mp != InitPlayer::SKILLENABLE && player->GetPlayerState() != EPlayerState::STRONGER )
								{
									mp = min( mp + InitPlayer::MPCOUNT, InitPlayer::SKILLENABLE );
									player->SetMp( mp );

									Packet::PlayerMpUpdate mpPacket( player->GetId(), mp );
									player->SendPacket( mpPacket );
								}

								// 발판 충돌 플레이어 점수 상승
								const unsigned char newPlayerScore = player->GetScore() + 1;

								player->SetScore( newPlayerScore );

								const Packet::Score upScore( player->GetId(), newPlayerScore );
								const Packet::Score downScore( basePlayer, basePlayerScore );

								// 같은 방에 있는 플레이어들에게 정보 전송
								for ( const auto index : players )
								{
									// 충돌 정보 전송
									Packet::CollisionTile collisionPacket = Packet::CollisionTile( player->GetId(), blockIndex );
									UserManager::GetInstance().GetUser( index )->SendPacket( collisionPacket );

									// 점수 상승 정보 전송
									UserManager::GetInstance().GetUser( index )->SendPacket( upScore );
									// 점수 하락시킬 플레이어가 있을 경우 해당 정보 전송
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
		WARN_LOG( "존재하지 않는 유저입니다. [ socket :  %d ]", socket );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	if ( !player )
	{
		WARN_LOG( "존재하지 않는 유저입니다. [ socket :  %d ]", socket );
		return;
	}
	const Packet::SkillUseResult result(player->GetId(), ServerToClient::SKILLUSE_REQUEST_FAILED);

	//PRINT_LOG( "스킬 사용 요청 패킷 송신" );
	if ( player->GetMp() == InitPlayer::SKILLENABLE )
	{
		int roomNum = player->GetRoomNum();
		RoomManager::GetInstance().PushTask(
		[ roomNum, socket ]( )
		{
			RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
			if ( !room )
			{
				WARN_LOG( "존재하지 않는 방입니다. [ roomNum :  %d ]", roomNum );
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

				INFO_LOG( "스킬 사용 승인 패킷 전송 [ socket : %d ]", socket );
			} );
		} );

		return;
	}
	
	//사용 불가 패킷
	player->SendPacket( result );
	//PRINT_LOG( "스킬 사용 불허 패킷 전송" );
}

void UserManager::ProcessMatchingRequest( const SOCKET socket, char* packet )
{
	//const Packet::MatchingRequest send = *reinterpret_cast< Packet::MatchingRequest* > ( packet );

	// 플레이어 로그인 후 3명 존재 시 바로 게임으로 넘어가도록
	// 방으로 이동
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

		// 방 새로 생성 - 3명 이하인 방이 없을 시 
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

		// 방에 접속 유저 추가
		currentRoom->PushPlayer( socket );

		// 유저 상태 갱신 및 현재 접속한 방 입력
		UserManager::GetInstance().PushTask(
		[ socket, roomNum ]()
		{
			UserManager::GetInstance().GetUser( socket )->SetClientState( EClientState::MATCHING );
			UserManager::GetInstance().GetUser( socket )->SetRoomNumber( roomNum );
		} );

		// 현재 방에 3명 존재 시 게임 시작
		if ( currentRoom->GetPlayers().size() == InitWorld::INGAMEPLAYER_NUM )
		{
			INFO_LOG( "방에 3명 입장 [ roomNum : %d ]", roomNum );
			currentRoom->SetState( RoomState::GAME );

			// 방에 있는 플레이어들에게 각각의 플레이어들 초기 정보 전송 (고유 색, 이름 등)
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
						// 인게임 플레이어 초기화 정보 클라이언트에게 보내기
						Packet::InitPlayers packet( users[ other ]->GetId() );
						packet.color = count;

						const Position pos = Position( InitPlayer::INITPOSITION_X[ count - 1 ], InitPlayer::INITPOSITION_Y[ count - 1 ] );
						//위치 수정하기
						packet.x = pos.x;
						packet.y = pos.y;
						packet.directionX = InitPlayer::INITDIRECTION_X[ count - 1 ];
						packet.directionY = InitPlayer::INITDIRECTION_Y[ count - 1 ];
						strcpy_s( packet.name, DataBaseManager::GetInstance().EncodingString( users[ other ]->GetName().c_str() ).c_str() );

						users[ other ]->SetPosition( pos );
						users[ other ]->SetColor( count );
						users[ index ]->SendPacket( packet );

						//초기 시작 타일 색 전송
						Packet::CollisionTile tile( packet.owner, InitWorld::FIRSTTILE_INDEX[ count - 1 ] );
						users[ index ]->SendPacket( tile );

						++count;
					}
				}

				INFO_LOG( "게임시작 패킷 전송 완료" );

				for ( const auto index : others )
				{
					users[ index ]->SetClientState( EClientState::GAME );

				}

				RoomManager::GetInstance().PushTask(
				[ roomNum ]()
				{
					//타이머 시작
					RoomManager::GetInstance().PushTimer( roomNum );

					//초기 시작 타일 색 변경
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
		WARN_LOG( "존재하지 않는 유저입니다. [ socket : %d ]", socket );
		return;
	}

	int id = user->GetId();
	int roomNum = user->GetRoomNum();
	EClientState state = user->GetClientState();

	if ( state != EClientState::GAMEFINISH )
	{
		WARN_LOG( "게임 룸 속 게임이 종료된 유저가 아닙니다. [ socket : %d ]", socket );
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
			INFO_LOG( "방에 존재한 플레이여 0명 - 방 삭제 [ roomNum : %d ]", roomNum );
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

	INFO_LOG( "QuitRoom 패킷 - 방 나가기 완료 [ roomNum : %d ]", roomNum );
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
	// TODO: 여기에 return 문을 삽입합니다.
	int id = -1;
	if ( !m_pIdPools.try_pop( id ) )
	{
		id = static_cast< int >( m_pIdPools.unsafe_size() );
		++id;
	}
	return id;
}
