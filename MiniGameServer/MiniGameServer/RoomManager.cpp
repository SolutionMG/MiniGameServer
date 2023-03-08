#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "MathManager.h"

RoomManager::RoomManager()
	:m_updateRoomTimers(), m_timerThread()
{
	m_rooms.reserve( InitServer::MAX_ROOMSIZE );

	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		m_roomNumberPools.push( i );
	}

	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		RoomUnit* room = new RoomUnit;
		room->Initialize();
		m_roomPools.push( room );
	}
}

RoomManager::~RoomManager()
{
	for ( auto& [index, room] : m_rooms )
	{
		delete room;
		room = nullptr;
	}
	m_rooms.clear();

	RoomUnit* room = nullptr;
	while ( m_roomPools.try_pop( room ) )
	{
		delete room;
		room = nullptr;
	}
}

void RoomManager::RunTimer()
{
	//타이머 만들기
	m_timerThread = static_cast< std::jthread >
		( [ this ]( std::stop_token stoken )
		{
			while ( !stoken.stop_requested() )
			{
				UpdateRoomTimer();
				// 1초마다 작동하는 타이머
				std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( 1000 ) );
			}

		} );	
}

void RoomManager::UpdateRoomTimer()
{
	// 각 플레이어에 시간 send
	for(auto& roomNum : m_updateRoomTimers )
	{
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		RoomUnit* room = m_rooms[ roomNum ];
		unsigned char time = room->GetTime();
		room->SetTime( ++time );

		//각 방의 플레이어들에게 타이머 Send
		auto& players = room->GetPlayers();
		Packet::Timer packet( time );

		for ( auto& player : players )
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser( player );
			if ( !user )
				continue;

			user->SendPacket( packet );
		}

		if ( time > 0 && ( time % InitWorld::ITEMSPAWNTIME == 0 ) )
		{
			// 해당 방에 아이템 추가 및 클라이언트에게 아이템 위치 및 타입 전송
			int randomPositionIndex = MathManager::GetInstance().randomInteger( 0, 48 );
			int itemType = MathManager::GetInstance().randomInteger( 0, ItemTypes::ITEMTYPES_SIZE );
			const Tile tile = room->GetTile( randomPositionIndex );
			int itemIndex = static_cast< int >( room->GetItems().size() );

		
			Packet::ItemSpawn itemSpawn( tile.x, tile.y, static_cast< unsigned char >( itemType ), itemIndex );
			// room객체에 아이템 정보 push
			Item newItem( tile.x, tile.y, static_cast< unsigned char >( itemType ), itemIndex );
			room->PushItem( newItem );

			for ( auto& player : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( player );
				if ( !user )
					continue;
				// 봉인
				//user->SendPacket( itemSpawn );
			}
			std::cout << roomNum << "방 " << itemType << "번 아이템 스폰" << std::endl;
		}

		if ( time == InitWorld::ENDGAMETIME )
		{
			// 게임 종료
			// 타이머에서 해당 방 삭제, 게임 종료 패킷 각 플레이어들에게 전송
			room->SetTime( 0 );
			m_deleteRoomTimers.emplace_back( roomNum );

			Packet::EndGame finalinfo;
			int index = 0;

			for ( auto& player : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( player );
				if ( !user )
					continue;

				//게임 종료 패킷 전송
				finalinfo.playerInfo[ index ].owner = user->GetId();
				finalinfo.playerInfo[ index++ ].score = user->GetScore();
			}

			for ( auto& player : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( player );
				if ( !user )
					continue;

				//게임 종료 패킷 전송
				//봉인
				//user->SendPacket( finalinfo );
			}
			std::cout << "게임 종료 시간 도달" << std::endl;
		}
	}

	// 100초가 넘었을 때 타이머에서 제거 (게임 종료)
	for ( auto& roomNum : m_deleteRoomTimers )
	{
		std::erase_if( m_updateRoomTimers,
			[ roomNum ]( const int& index )
			{ return index == roomNum; });
	}
	m_deleteRoomTimers.clear();
}

void RoomManager::PushRoomNumber( const int& number )
{
	m_roomNumberPools.push( number );
}

void RoomManager::PushTimer( const int& roomNum )
{
	m_updateRoomTimers.emplace_back( roomNum );
}

void RoomManager::PushRoom( RoomUnit* room )
{
	if ( !room )
		return;
	room->Initialize();
	m_roomPools.push( room );
}

RoomUnit* RoomManager::GetRoomUnitFromPools()
{
	RoomUnit* room = nullptr;
	if ( !m_roomPools.try_pop( room ) )
	{
		room = new RoomUnit;
		room->Initialize();
	}
	return room;
}

RoomUnit* RoomManager::GetRoom( const int& index )
{
	if ( m_rooms.find( index ) == m_rooms.end() )
	{
		return nullptr;
	}
	return m_rooms[ index ];
}

const int RoomManager::GetNewRoomNumber( )
{
	int room = -1;
	if ( !m_roomNumberPools.try_pop( room ) )
	{
		room = static_cast< int >( m_roomNumberPools.unsafe_size( ) ) + 1;
	}
	return room;
}

void RoomManager::DeleteRoom( const int& index )
{
	m_rooms.erase( index );
}
