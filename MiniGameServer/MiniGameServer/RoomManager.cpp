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

		auto& room = m_rooms[ roomNum ];
		unsigned char time = room.GetTime();
		room.SetTime( ++time );

		//각 방의 플레이어들에게 타이머 Send
		for ( auto& player : m_rooms[ roomNum ].GetPlayers() )
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser( player );
			if ( !user )
				continue;

			Packet::Timer packet( time );
			user->SendPacket( packet );
		}

		if ( time > 0 && ( time % InitWorld::ITEMSPAWNTIME == 0 ) )
		{
			//해당 방에 아이템 추가 및 클라이언트에게 아이템 위치 및 타입 전송

			//int randomIndex = MathManager::GetInstance().randomInteger( 0, 48 );
			//int itemType = MathManager::GetInstance().randomInteger( 0, 3 );
			//const Tile tile = room.GetTile( randomIndex );
			//Packet::ItemSpawn item( tile.x, tile.y, static_cast< unsigned char >( itemType ) );
			//itemspawn 플레이어들에게 send, room객체에도 아이템 정보 push
		}

		if ( time == 100 )
		{
			room.SetTime( 0 );
			m_deleteRoomTimers.emplace_back( roomNum );
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

RoomUnit& RoomManager::GetRoom( const int& index )
{
	if ( m_rooms.find( index ) == m_rooms.end() )
	{
		RoomUnit trash;
		trash.SetTileColor( 0, -1 );
		return trash;
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
