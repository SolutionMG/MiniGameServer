#ifndef OVERLAPPED_H
#define OVERLAPPED_H

#include "Singleton.hpp"
#include <concurrent_queue.h>

class OverlappedManager final
	: public Base::TSingleton < OverlappedManager >
{
public:
	explicit OverlappedManager( );
	virtual ~OverlappedManager( );

	// Set
	void PushOverlapped( WSAOVERLAPPED_EXTEND* over );

	// Get
	WSAOVERLAPPED_EXTEND* GetOverlapped( );

private:
	concurrency::concurrent_queue<WSAOVERLAPPED_EXTEND*> m_overlappedPools;

};

#endif // !OVERLAPPED_H
