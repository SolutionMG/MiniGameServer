#include "pch.h"
#include "BaseTaskManager.h"
#include "Log.h"

BaseTaskManager::BaseTaskManager()
    : m_taskQueue(), m_taskThread()
{
}

BaseTaskManager::~BaseTaskManager() = default;

void BaseTaskManager::PushTask( const Task& task )
{
    m_taskQueue.push( task );
}

void BaseTaskManager::Run()
{
    m_taskThread = static_cast< std::jthread >
        ( [ this ]( std::stop_token stoken )
        {
            while ( !stoken.stop_requested() )
            {
                WorkTask();
                std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( InitServer::UPDATE_AWAKE_MS ) );
            }
        } );
}

void BaseTaskManager::WorkTask()
{
    Task task = nullptr;
    while ( m_taskQueue.try_pop( task ) )
    {
        task();
    }
}
