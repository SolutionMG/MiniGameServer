#ifndef BASETASK_H
#define BASETASK_H

#include <concurrent_queue.h>
#include <thread>
#include <functional>

class BaseTaskManager
{
public:
	using Task = std::function<void( void )>;
public:
	explicit BaseTaskManager( );
	virtual ~BaseTaskManager( ) noexcept;
	
	
	void PushTask( const Task& task );
	virtual void Run( );

protected:
	virtual const std::string GetName( )	const  noexcept = 0;
	virtual const int GetAwakeInterval( )	const  noexcept = 0;

protected:
	void WorkTask( );

protected:
	concurrency::concurrent_queue< Task >	m_taskQueue;
	std::jthread							m_taskThread;
};

#endif // !BASETASK_H
