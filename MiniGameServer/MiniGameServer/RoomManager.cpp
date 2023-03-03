#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"

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
	int roomNum = -1;
	while ( m_updateRoomTimers.try_pop( roomNum ) )
	{
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		auto& room = m_rooms[ roomNum ];
		room.SetTime( room.GetTime() + 1 );

		//�� ���� �÷��̾�鿡�� Ÿ�̸� Send
	}
}

void RoomManager::PushRoomNumber( const int& number )
{
	m_roomPools.push( number );
}

void RoomManager::PushTimer( const int& roomNum )
{
	m_updateRoomTimers.push( roomNum );
}

const int& RoomManager::GetNewRoomNumber( )
{
	int room = -1;
	if ( !m_roomPools.try_pop( room ) )
	{
		room = m_roomPools.unsafe_size() + 1;
	}
	return room;
}
