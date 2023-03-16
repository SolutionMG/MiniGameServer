/*
	Copyright 2021, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

// Dev Debug Log
// #define WONSY_DEBUG_LOG

#include "WonSY_Log.h"

//#include "WonSY_Attributes.hh"
//#include "WonSY_AutoCall.hpp"
//#include "WonSY_Time.h"

#include <iostream>
#include <functional>
#include <thread>
#include <ctime>
#include <iomanip>
#include <chrono>

#ifdef WONSY_CPP_20_OVER
#include <format>
#else
#include <sstream>
#endif

#ifndef WONSY_UNREAL
	#ifdef WONSY_WINDOW
		//#include "WonSY_CommonWindowsHeader.h"
	#endif
#else
	#include "CoreMinimal.h"
#endif

#ifdef WONSY_LOG_LOCK
	#include <shared_mutex>
	static std::shared_mutex logLock;
#endif

#ifdef WONSY_LOG_TASK_RUNNER
WonSY::Log::LogManager::LogManagerContextMemory::~LogManagerContextMemory() 
{ 
	delete m_writeFilePtr; 
};
#endif

WonSY::Log::LogManager::LogManager()
#ifdef WONSY_LOG_TASK_RUNNER
	: WsyTaskRunner< WsyLogManagerContext >( "LogManager", UPDATE_COOLTIME, 1 )
	, m_lmcm()
#endif
{
#ifdef WONSY_LOG_WRITE_FILE
	m_lmcm.Ref( WsyLogManagerContext() ).m_fileName = std::move( m_initLogFileName );
#endif
}

WonSY::Log::LogManager::~LogManager() = default;

#ifdef WONSY_LOG_WRITE_FILE
void WonSY::Log::LogManager::DoPreInit( std::string&& fileName ) noexcept
{
	LogManager::m_initLogFileName = std::move( fileName );
}

void WonSY::Log::LogManager::_SetLogFileName( const std::string& fileName )
{
// todo : 인터페이스 너무 구리고, 이거 억지인데;; 후..
	PushTask( [ fileName ]( WsyLogManagerContextRef key ) mutable
		{
			if ( fileName == LogManager::GetInstance().Ref( key ).m_fileName )
				return;

			INFO_LOG(
				"Change Log File Name! [ oldfileName : , " + LogManager::GetInstance().Ref( key ).m_fileName +
				", newfileName : , " + fileName );

			LogManager::GetInstance().Ref( key ).m_fileName     = fileName;
			LogManager::GetInstance().Ref( key ).m_writeFilePtr = nullptr;
		} );
}
#endif