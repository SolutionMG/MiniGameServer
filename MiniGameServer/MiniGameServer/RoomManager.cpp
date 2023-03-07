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
	//Ÿ�̸� �����
	m_timerThread = static_cast< std::jthread >
		( [ this ]( std::stop_token stoken )
		{
			while ( !stoken.stop_requested() )
			{
				UpdateRoomTimer();
				// 1�ʸ��� �۵��ϴ� Ÿ�̸�
				std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( 1000 ) );
			}

		} );
			
}

void RoomManager::UpdateRoomTimer()
{
	// �� �÷��̾ �ð� send
	for(auto& roomNum : m_updateRoomTimers )
	{
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		auto& room = m_rooms[ roomNum ];
		unsigned char time = room.GetTime();
		room.SetTime( ++time );

		//�� ���� �÷��̾�鿡�� Ÿ�̸� Send
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
			//�ش� �濡 ������ �߰� �� Ŭ���̾�Ʈ���� ������ ��ġ �� Ÿ�� ����

			//int randomIndex = MathManager::GetInstance().randomInteger( 0, 48 );
			//int itemType = MathManager::GetInstance().randomInteger( 0, 3 );
			//const Tile tile = room.GetTile( randomIndex );
			//Packet::ItemSpawn item( tile.x, tile.y, static_cast< unsigned char >( itemType ) );
			//itemspawn �÷��̾�鿡�� send, room��ü���� ������ ���� push
		}

		if ( time == 100 )
		{
			room.SetTime( 0 );
			m_deleteRoomTimers.emplace_back( roomNum );
		}
	}

	// 100�ʰ� �Ѿ��� �� Ÿ�̸ӿ��� ���� (���� ����)
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
