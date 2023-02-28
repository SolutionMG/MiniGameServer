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

const int& RoomManager::GetNewRoomNumber( )
{
	int room = -1;
	m_roomPools.try_pop( room );
	return room;
}
