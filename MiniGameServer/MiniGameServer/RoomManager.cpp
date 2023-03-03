#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"

RoomManager::RoomManager( )
{
	m_rooms.reserve( InitServer::MAX_ROOMSIZE );
	
	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		m_roomPools.push( i );
	}
}

void RoomManager::PushRoomNumber( const int& number )
{
	m_roomPools.push( number );
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
