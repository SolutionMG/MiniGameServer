#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"
#include "UserManager.h"
#include "PlayerUnit.h"

RoomManager::RoomManager( )
	:m_updateRoomTimers(), m_timerThread()
{
	m_rooms.reserve( InitServer::MAX_ROOMSIZE );
	
	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		m_roomPools.push( i );
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
		auto& users = UserManager::GetInstance().GetUsers();
		for ( auto& player : m_rooms[ roomNum ].GetPlayers() )
		{
			if ( users.find( player ) == users.end() )
				continue;

			Packet::Timer packet( time );
			users[ player ]->SendPacket( packet );
		}

		if ( time == 100 )
		{
			room.SetTime( 0 );
			m_deleteRoomTimers.emplace_back( roomNum );
		}
	}

	// 93�ʰ� �Ѿ��� �� Ÿ�̸ӿ��� ���� (���� ����)
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
	m_roomPools.push( number );
}

void RoomManager::PushTimer( const int& roomNum )
{
	m_updateRoomTimers.emplace_back( roomNum );
}

const int RoomManager::GetNewRoomNumber( )
{
	int room = -1;
	if ( !m_roomPools.try_pop( room ) )
	{
		room = static_cast< int >( m_roomPools.unsafe_size( ) ) + 1;
	}
	return room;
}
