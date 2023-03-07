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
		player->SetName( "Default" );
		player->SetState( EClientState::DISCONNECT );
		player->SetPosition( Position( 0.f, 0.f ) );
		player->SetRoomNumber( -1 );
		player->SetId( -1 );
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
		//return;
	}

	PlayerUnit* player = m_users[ socket ];

	player->SetPosition( currentPos );
	const short color = player->GetColor();

	int roomNum = player->GetRoomNum();
	auto& room = RoomManager::GetInstance().GetRoom( roomNum );

	if ( room.GetTile(0).color == -1 )
		return;

	auto& players = room.GetPlayers();
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
		const Tile& tile = room.GetTile( blockIndex );

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
						basePlayerScore = m_users[ index ]->GetScore();
						if ( basePlayerScore > 0 )
						{
							basePlayer = m_users[ index ]->GetId();
							m_users[ index ]->SetScore( --basePlayerScore );
						}
						break;
					}
				}
			}
			
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

				//// 점수 상승 정보 전송
				//m_users[ index ]->SendPacket( upScore );

				//// 점수 하락시킬 플레이어가 있을 경우 해당 정보 전송
				//if ( basePlayer != -1 )
				//{
				//	m_users[ index ]->SendPacket( downScore );
				//}

			}

			//타일 색 충돌 플레이어 색으로 변경
			RoomManager::GetInstance().PushTask(
				[ blockIndex, roomNum, color ]()
				{
					auto& room = RoomManager::GetInstance().GetRoom( roomNum );
					if(room.GetTile(0).color == -1)
						return;

					room.SetTileColor( blockIndex, color );

				} );
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

	player->SetName( "default" );
	player->SetState( EClientState::DISCONNECT );
	player->SetPosition( Position( 0.f, 0.f ) );
	player->SetRoomNumber( -1 );
	player->SetId( -1 );

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
		player->SetName( "default" );
		player->SetState( EClientState::DISCONNECT );
		player->SetPosition( Position( 0.f, 0.f) );
		player->SetRoomNumber( -1 );
		player->SetId( -1 );
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
