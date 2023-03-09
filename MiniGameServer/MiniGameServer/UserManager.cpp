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
		PRINT_LOG( "socket == null, 존재하지 않는 유저로부터의 패킷입니다." );
		return;
	}

	if ( m_processFunctions.find( packet[ 1 ] ) == m_processFunctions.end() )
	{
		PRINT_LOG( "없는 패킷 타입입니다." );
		return;
	}

	m_processFunctions[ packet[ 1 ] ]( socket, packet );
}

void UserManager::AddProcess( )
{
	m_processFunctions.reserve( 10 );
	m_processFunctions.emplace( std::make_pair( ClientToServer::LOGIN_REQUEST, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void { return ProcessLoginRequest( socket, packet ); } ) ));
	m_processFunctions.emplace( std::make_pair( ClientToServer::MOVE, std::function( [ & ]( const SOCKET& socket, char* packet ) -> void { return ProcessMove( socket, packet ); } ) ) );
}

void UserManager::ProcessLoginRequest( const SOCKET& socket, char* packet )
{
	// 로그인 요청 후 
	int id = m_users[ socket ]->GetId();

	Packet::LoginRequest data = *reinterpret_cast< Packet::LoginRequest* > ( packet );
	int baseScore = 0;
	if ( DataBaseManager::GetInstance().LogOn( data.name, data.password, baseScore ) )
	{
		Packet::LoginResult send( id, ServerToClient::LOGON_OK );
		strcpy_s( send.name, data.name );
		m_users[ socket ]->SendPacket( send );
	}
	else
	{
		Packet::LoginResult send( id, ServerToClient::LOGON_FAILED );
		strcpy_s( send.name, data.name );
		m_users[ socket ]->SendPacket( send );
	}
}

void UserManager::ProcessMove( const SOCKET& socket, char* packet )
{
	Packet::Move send = *reinterpret_cast< Packet::Move* > ( packet );
	send.info.type = ServerToClient::MOVE;

	//검증
	Position previousPos = m_users[ socket ]->GetPosition();
	Position currentPos = Position( send.x, send.y );

	float distance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, currentPos.x, currentPos.y );
	float predictDistance = MathManager::GetInstance().Distance2D( previousPos.x, previousPos.y, previousPos.x * InitServer::MAX_DISTANCE, previousPos.y * InitServer::MAX_DISTANCE );

	if ( distance > predictDistance )
	{
		PRINT_LOG( "이상한 좌표를 수신" );
		return;
	}

	PlayerUnit* player = m_users[ socket ];

	player->SetPosition( currentPos );
	const short color = player->GetColor();

	int roomNum = player->GetRoomNum();
	RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );

	if ( !room )
	{
		PRINT_LOG( "존재하지 않는 방입니다." );
		return;
	}
	auto& players = room->GetPlayers();
	// 이동
	{
		// 같은 방에 있는 플레이어들에게 정보 전송
		for ( const auto& index : players )
		{
			if ( index == socket )
				continue;

			m_users[ index ]->SendPacket( send );
		}
	}

	// 벽 충돌
	{
		unsigned char returnValue = MathManager::GetInstance().CheckCollisionWall( currentPos.x, currentPos.y );
		if ( returnValue != InitWorld::NOTWALLCOLLISION)
		{
			Packet::CollisionWall wall( player->GetId(), returnValue );
			
			for ( const auto& index : players )
			{
				m_users[ index ]->SendPacket( wall );
			}
		}
	}

	// 플레이어 간 충돌
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

			//충돌 검사
			const Position temp = m_users[ index ]->GetPosition();
			//std::cout << player->GetColor() << "색상 플레이어와 " << m_users[ index ]->GetColor() << "색상 플레이어 충돌 검사" << std::endl;
			if ( MathManager::GetInstance().CollisionSphere( currentPos.x, currentPos.y, temp.x, temp.y ) )
			{
				collision = true;

				if ( count < 1 || count > 2 )
					continue;

				cp.owners[ count++ ] = m_users[ index ]->GetId();

				// 1 빨강 2 파랑 3 노랑
				// std::cout << "!!!!!!!!" << player->GetColor() << "색상 플레이어와 " << m_users[ index ]->GetColor() << "색상 플레이어 충돌 발생!!!!!!!!" << std::endl;

			}
		}

		// 플레이어 간 충돌 사실 전달
		if ( collision )
		{
			for ( const auto& index : players )
			{
				m_users[ index ]->SendPacket( cp );
			}
		}
	}

	// 아이템과 플레이어 충돌
	{
		auto& items = room->GetItems();
		std::vector<Packet::ItemUse> collisionItems;
		collisionItems.reserve( static_cast< int >( ( InitWorld::ENDGAMETIME / InitWorld::ITEMSPAWNTIME ) ) );

		for ( auto& item : items )
		{
			if ( MathManager::GetInstance().CollisionSphere(currentPos.x, currentPos.y, item.x, item.y, (InitWorld::ITEM_SIZE / 2.f) + (InitWorld::PLAYERCOLLIDER / 2.f) ) )
			{
				// 플레이어와 아이템 충돌
				// 해당 정보 플레이어들에게 전송 ( 어떤 아이템과 어떤 플레이어가 충돌했는 가)
				Packet::ItemUse usingItem(player->GetId(), item.itemType, item.index);
				collisionItems.emplace_back( usingItem );
			}
		}

		if ( !collisionItems.empty() )
		{
			//충돌한 아이템 정보 send
			for ( auto& index : players )
			{
				for ( auto& usingItem : collisionItems )
				{
					//봉인
					//m_users[ index ]->SendPacket( usingItem );
				}
			}

			// 사용된 아이템 방에서 삭제
			RoomManager::GetInstance().PushTask(
				[ collisionItems, roomNum ]()
				{
					RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
					
					if ( !room )
						return;
					// 충돌된 아이템 
					for ( auto& item : collisionItems )
					{
						room->PopItem( item.itemIndex );
					}
					std::cout << "사용된 아이템 삭제" << std::endl;
				} );
		}
	}
	
	// 발판 충돌
	{
		int xIndex = static_cast< int >( ( currentPos.x - ( InitWorld::FIRST_TILEPOSITION_X - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //타일의 X인덱스
		int yIndex = static_cast< int >( ( currentPos.y - ( InitWorld::FIRST_TILEPOSITION_Y - InitWorld::TILEWITHGAP_SIZE / 2.f ) ) / ( InitWorld::TILEWITHGAP_SIZE ) ); //타일의 Y인덱스
		int blockIndex = xIndex + ( yIndex * InitWorld::TILE_COUNTX );

		if ( blockIndex < 0 || blockIndex > 48 )
		{
			PRINT_LOG( "맵 외부의 좌표 수신" );
			return;
		}

		Tile tile;
		RoomManager::GetInstance().PushTask(
			[&tile, blockIndex, roomNum ]()
			{
				const Tile temp = RoomManager::GetInstance().GetRooms()[ roomNum ]->GetTile(blockIndex);
				tile.index = temp.index;
				tile.color = temp.color;
				tile.x = temp.x;
				tile.y = temp.y;
			} );

	

		if ( tile.color == player->GetColor() )
			return;
		

		if ( MathManager::GetInstance().CollisionPointAndRectangle( currentPos.x, currentPos.y, tile.x, tile.y) )
		{
			//std::cout << player->GetId() << "의 충돌 정보 전송" << blockIndex << std::endl;

			unsigned char basePlayerScore = 0;
			int basePlayer = -1;

			//하얀 색 발판이면 점수를 하락시킬 플레이어가 없음
			if ( tile.color != 0 )
			{
				// 기존 색상 플레이어 점수 하락
				for ( auto& index : players )
				{
					if ( m_users[ index ]->GetColor() == tile.color )
					{
						// 해당 플레이어 점수는 정상적이라면 1점 이상이어야 함.
						basePlayerScore = m_users[ index ]->GetScore();

						if ( basePlayerScore == 0 )
						{
							PRINT_LOG( "점수 오차 발생" );
							std::cout << "점수: " << m_users[ index ]->GetScore() << std::endl;
							std::cout << index << " 플레이어" << std::endl;
							break;
						}

						basePlayer = m_users[ index ]->GetId();
						m_users[ index ]->SetScore( --basePlayerScore );
						break;
					}
				}
			}
			
			//타일 색 충돌 플레이어 색으로 변경
			RoomManager::GetInstance().PushTask(
				[ blockIndex, roomNum, color ]()
				{
					RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
					if ( !room )
					{
						PRINT_LOG( "존재하지 않는 방입니다." );
						return;
					}
					room->SetTileColor( blockIndex, color );

				} );

			// 발판 충돌 플레이어 점수 상승
			unsigned char newPlayerScore = player->GetScore() + 1;
			player->SetScore( newPlayerScore );

			Packet::Score upScore( player->GetId(), newPlayerScore );
			Packet::Score downScore( basePlayer, basePlayerScore );

			// 같은 방에 있는 플레이어들에게 정보 전송
			for ( const auto& index : players )
			{
				// 충돌 정보 전송
				Packet::CollisionTile collisionPacket = Packet::CollisionTile( send.owner, blockIndex );
				m_users[ index ]->SendPacket( collisionPacket );

				//봉인
				//// 점수 상승 정보 전송
				//m_users[ index ]->SendPacket( upScore );

				//// 점수 하락시킬 플레이어가 있을 경우 해당 정보 전송
				//if ( basePlayer != -1 )
				//{
				//	m_users[ index ]->SendPacket( downScore );
				//}

			}
		}
	}
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
	// TODO: 여기에 return 문을 삽입합니다.
	int id = -1;
	if ( !m_pIdPools.try_pop( id ) )
	{
		id = static_cast< int >( m_pIdPools.unsafe_size() );
		++id;
	}
	return id;
}
