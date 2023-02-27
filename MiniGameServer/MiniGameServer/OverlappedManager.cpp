#include "pch.h"
#include "OverlappedManager.h"

OverlappedManager::OverlappedManager( )
{
	for ( int i = 0; i < InitServer::OVERLAPPED_SIZE; ++i )
	{
		WSAOVERLAPPED_EXTEND* over = new WSAOVERLAPPED_EXTEND;
		ZeroMemory( &over->over, sizeof( over->over ) );
		over->type = EOperationType::SEND;

		m_overlappedPools.push( over );
	}
}

OverlappedManager::~OverlappedManager( )
{
	WSAOVERLAPPED_EXTEND* over = nullptr;

	while( m_overlappedPools.try_pop( over ) )
	{
		delete over;
		over = nullptr;
	}
}

void OverlappedManager::PushOverlapped( WSAOVERLAPPED_EXTEND* over )
{
	ZeroMemory( &over->over, sizeof( over->over ) );
	over->type = EOperationType::SEND;
	m_overlappedPools.push( over );
}

WSAOVERLAPPED_EXTEND* OverlappedManager::GetOverlapped( )
{
	WSAOVERLAPPED_EXTEND* over = nullptr;
	if ( !m_overlappedPools.try_pop( over ) )
	{
		over = new WSAOVERLAPPED_EXTEND;
		ZeroMemory( &over->over, sizeof( over->over ) );
		over->type = EOperationType::SEND;
	}

	return over;
}
