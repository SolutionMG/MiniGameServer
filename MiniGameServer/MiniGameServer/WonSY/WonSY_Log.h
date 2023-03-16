/*
	Copyright 2020, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once

#ifndef WONSY_LOG
#define WONSY_LOG

//#include "WonSY_Macro.hh"
#include "WonSY_Singleton.hpp"
//#include "WonSY_Type.hh"
#include "WonSY_Time.h"

#include <string>
#include <array>
#include <source_location>
#include <iostream>

#include <Windows.h>

// LogManager�� ���ۿ� ���� �Ʒ��� ��ũ�ΰ� ��ó���� ������ ���ǵǾ� �־�� �մϴ�.
//		- WONSY_LOG_TASK_RUNNER : �α׸� TaskRunner�� ���� ó���� ���, �����մϴ�.
//		- WONSY_LOG_WRITE_FILE  : WONSY_LOG_TASK_RUNNER�� Ȱ��ȭ�Ǿ� ���� ��,      �α� ������ ���� ���, �����մϴ�.
//		- WONSY_LOG_LOCK        : WONSY_LOG_TASK_RUNNER�� Ȱ��ȭ�Ǿ� ���� ���� ��, Lock�� ����Ͽ� �α׸� ó���� ���, �����մϴ�.
//		- WONSY_LOG_LEVEL_ALL   : �׻� ��� �α׸� ����ϰ� ���� ���, �����մϴ�.

// �ӽ�
#define WONSY_WINDOW
#define WONSY_LOG_LOCK

// �⺻�����δ� �������� ���� ��ũ�θ� �������ִ°� ������, WonSY Proj Type�� ���ǵ� ��쿡�� Defaultň �־��ݴϴ�.
#ifdef WONSY_SERVER
#ifndef WONSY_LOG_FORCE_DISABLE_ADDITIONAL
	#define WONSY_LOG_TASK_RUNNER
	#define WONSY_LOG_WRITE_FILE
#endif
#elif WONSY_UNREAL
	
#elif WONSY_CLIENT
	#define WONSY_LOG_TASK_RUNNER
	#define WONSY_LOG_WRITE_FILE
#elif WONSY_CORE
	#define WONSY_LOG_LOCK
#endif

#ifdef WONSY_LOG_LOCK
	#include <shared_mutex>
	static std::shared_mutex s_logLock;
#endif

#ifdef WONSY_LOG_WRITE_FILE
	#ifndef WONSY_LOG_TASK_RUNNER
		static_assert( false, "�Ұ����� �����Դϴ�. ���Ͽ� ���� ���ؼ��� WONSY_LOG_TASK_RUNNER�� �ʿ�� �մϴ�." );
	#endif
#endif

#ifdef WONSY_LOG_LOCK
	#ifdef WONSY_LOG_TASK_RUNNER
		static_assert( false, "�Ұ����� �����Դϴ�. ���� ����� ���, WONSY_LOG_TASK_RUNNER�� ������� ������." );
	#endif
#endif

#ifdef WONSY_LOG_TASK_RUNNER
#include "WonSY_Key.h"
#include "WonSY_TaskRunner.hpp"
#include "WonSY_ContextPtr.hpp"
#include "WonSY_File.h"
#endif

#define WONSY_DEV_DEBUG_LOG_IS_ERROR 1

namespace WonSY
{
	// ���漱��j

#ifdef	WONSY_LOG_WRITE_FILE
	namespace File
	{
		class FileWriter;
	}
#endif

	namespace Log
	{
#define LOG_LEVEL_ALL        0
#define LOG_LEVEL_NOT_COMMON 1
#define LOG_LEVEL_NOT_INFO   2
#define LOG_LEVEL_NOT_WARN   3
#define LOG_LEVEL_NOT_ERROR  4
#define LOG_LEVEL_NOT_ALL    5

#define LOG_LEVEL LOG_LEVEL_ALL

		enum class LOG_TYPE : unsigned char
		{
			LT_ERROR,
			LT_WARN,
			LT_INFO,
			LT_COMMON,
			None
		};

		class LogManager
			: public WonSY::Base::TSingleton< LogManager >
#ifdef WONSY_LOG_TASK_RUNNER
			, public WonSY::BaseTaskRunner< WsyLogManagerContext >
#endif
		{
		public:
#ifdef WONSY_LOG_TASK_RUNNER
			inline static constexpr int32       UPDATE_COOLTIME   = 100;

			class LogManagerContextMemory
			{
			public:
				~LogManagerContextMemory();

			public:
				WonSY::File::FileWriter* m_writeFilePtr = nullptr;
				int32                    m_year         = 0;
				int32                    m_month        = 0;
				int32                    m_day          = 0;
				int32                    m_time         = 0;
				std::string              m_fileName     = "";
			};
#endif

		private:
			inline static std::string m_initLogFileName = "";

		public:
			LogManager();
			~LogManager();

#ifdef WONSY_LOG_TASK_RUNNER
		public:
			template< LOG_TYPE logType >
			void AddLog( WonSY::Time::_WsyTime&&, std::string&& message );
			
			LogManagerContextMemory& Ref( WsyLogManagerContextRef );

		private:
			WsyContextPtr< WsyLogManagerContext, LogManagerContextMemory > m_lmcm;
#endif

#ifdef WONSY_LOG_WRITE_FILE
		public:
			static void DoPreInit( std::string&& fileName ) noexcept;

		private:
			void _SetLogFileName( const std::string& fileName );
#endif
		};

		template< LOG_TYPE logType, bool isDirectPrintLog, typename ... Args > void WriteLog( const char* fileName, std::source_location&& sourceLocation, std::string_view format, Args... args );

//#ifdef WONSY_CPP_20_OVER
//	//	template< class... _Types > void ErrorFormatLog ( const SourceLocation& sourceLocation, const std::string& message, _Types&&... _Args ) { ErrorLog ( sourceLocation, std::format( message, std::forward< _Types >( _Args )... ) ); }
//	//	template< class... _Types > void WarnFormatLog  ( const SourceLocation& sourceLocation, const std::string& message, _Types&&... _Args ) { WarnLog  ( sourceLocation, std::format( message, std::forward< _Types >( _Args )... ) ); }
//	//	template< class... _Types > void InfoFormatLog  ( const SourceLocation& sourceLocation, const std::string& message, _Types&&... _Args ) { InfoLog  ( sourceLocation, std::format( message, std::forward< _Types >( _Args )... ) ); }
//	//	template< class... _Types > void CommonFormatLog( const SourceLocation& sourceLocation, const std::string& message, _Types&&... _Args ) { CommonLog( sourceLocation, std::format( message, std::forward< _Types >( _Args )... ) ); }
//#endif

#ifdef WONSY_WINDOW
		template< LOG_TYPE logType > void SetLogColor();
		template< bool isTerminate, typename ... Args > void WindowsLog( std::source_location&& sourceLocation, std::string_view format, Args... args );

		void SetLogColor( const WonSY::Log::LOG_TYPE logType );
#endif
	}
}

template< WonSY::Log::LOG_TYPE logType, bool isDirectPrintLog, typename ... Args >
void WonSY::Log::WriteLog( const char* fileName, std::source_location&& sourceLocation, std::string_view format, Args... args )
{
	auto curTime = WonSY::Time::_WsyTime( WonSY::Time::INIT_MODE::CUR_TIME );

	std::string logString;
	{
		// todo : 100������ ����ϵ��� ���ѵǾ��µ�;; �ɱ� �׷��� �͆���;
		static constexpr int LOG_DEFAULT_SIZE = 100;
		//static_assert( format.size() > LOG_DEFAULT_SIZE / 2, "LOG_DEFAULT_SIZE expected to be small." );

		logString.reserve( LOG_DEFAULT_SIZE );

		// �α� Ÿ���� �߰��մϴ�.
		{
			     if constexpr ( WonSY::Log::LOG_TYPE::LT_ERROR  == logType ) { logString.append( "[ ERROR  | " ); }
			else if constexpr ( WonSY::Log::LOG_TYPE::LT_WARN   == logType ) { logString.append( "[ WARN   | " ); }
			else if constexpr ( WonSY::Log::LOG_TYPE::LT_INFO   == logType ) { logString.append( "[ INFO   | " ); }
			else if constexpr ( WonSY::Log::LOG_TYPE::LT_COMMON == logType ) { logString.append( "[ COMMON | " ); }
			else                                                             { logString.append( "[ OTHERS | " ); }
		}

		// �ð����� �߰��մϴ�.
		{
			logString.append( curTime.ToString() );
			logString.append( " | " );
		}

		// ���ϸ�� ���θ��� �߰��մϴ�.
		{
			logString.append( fileName );
			logString.append( " " );
			logString.append( "(" );
			logString.append( std::to_string( sourceLocation.line() ) );
			logString.append( ") ] " );
		}

		// ���˵� �α� ��Ʈ���� �߰��մϴ�.
		{
			// todo : �̺κп��� ���� ��ȿ������ ���� �ִµ�, retString ���⿡�ٰ� �ٷ� ó���� �� ������� ����
			const size_t strSize                 = snprintf( nullptr, 0, format.data(), args ...) + 1;
			const auto   coppiedSize             = strSize > LOG_DEFAULT_SIZE ? LOG_DEFAULT_SIZE : strSize; 
			      char   str[ LOG_DEFAULT_SIZE ] = "\0";

			snprintf( str, coppiedSize, format.data(), args ... );

			logString.append( str, str + coppiedSize );
		}
	}

#ifdef WONSY_UNREAL
	GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Red, *( FString( logMessage.c_str() ) ) );
	UE_LOG( LogTemp, Fatal, TEXT( "%s" ), logMessage.c_str() );

	if constexpr ( logType == LOG_TYPE::LT_ERROR )
	{
		abort();
	}

	return;
#else

	if constexpr ( isDirectPrintLog )
	{
#ifdef WONSY_LOG_LOCK
		s_logLock.lock();
#endif

#ifdef  WONSY_WINDOW
		SetLogColor< logType >();
#endif

		std::cout << logString << std::endl;

#ifdef  WONSY_WINDOW
		SetLogColor< LOG_TYPE::None >();
#endif

#ifdef WONSY_LOG_LOCK
		s_logLock.unlock();
#endif

		return;
	}

	#ifdef WONSY_LOG_TASK_RUNNER

		WonSY::Log::LogManager::GetInstance().AddLog< logType >( std::move( curTime ), std::move( logString ) );
		return;

	#else
		#ifdef WONSY_LOG_LOCK
			s_logLock.lock();
		#endif
	
		#ifdef  WONSY_WINDOW
				SetLogColor< logType >();
		#endif

		std::cout << logString << std::endl;
	
		#ifdef  WONSY_WINDOW
				SetLogColor< LOG_TYPE::None >();
		#endif
	
		#ifdef WONSY_LOG_LOCK
			s_logLock.unlock();
		#endif
	#endif
#endif
}

#ifndef WONSY_UNREAL
	#ifdef  WONSY_WINDOW
	template< WonSY::Log::LOG_TYPE logType >
	void WonSY::Log::SetLogColor()
	{
		unsigned short color;
	
		if constexpr ( logType == LOG_TYPE::None )
		{
			color = 0x0F | ( 0 << 4 );
		}
		else if constexpr ( logType == LOG_TYPE::LT_COMMON )
		{
			color = 0x08 | ( 0x00 << 4 );
		}
		else if constexpr ( logType == LOG_TYPE::LT_WARN )
		{
			color = 0x0F | ( 0x06 << 4 );
		}
		else if constexpr ( logType == LOG_TYPE::LT_ERROR )
		{
			color = 0x0F | ( 0x0C << 4 );
		}
		else
		{
			color = 0x0F | ( 0 << 4 );
		}
	
		SetConsoleTextAttribute( GetStdHandle( (DWORD)( -11 ) ), color );
	}

	//void WonSY::Log::SetLogColor( const WonSY::Log::LOG_TYPE logType )
	//{
	//	unsigned short color;
	//
	//	if ( logType ==  WonSY::Log::LOG_TYPE::None )
	//	{
	//		color = 0x0F | ( 0 << 4 );
	//	}
	//	else if ( logType ==  WonSY::Log::LOG_TYPE::LT_COMMON )
	//	{
	//		color = 0x08 | ( 0x00 << 4 );
	//	}
	//	else if ( logType ==  WonSY::Log::LOG_TYPE::LT_WARN )
	//	{
	//		color = 0x0F | ( 0x06 << 4 );
	//	}
	//	else if ( logType ==  WonSY::Log::LOG_TYPE::LT_ERROR )
	//	{
	//		color = 0x0F | ( 0x0C << 4 );
	//	}
	//	else
	//	{
	//		color = 0x0F | ( 0 << 4 );
	//	}
	//
	//	SetConsoleTextAttribute( GetStdHandle( (DWORD)( -11 ) ), color );
	//}
	#endif
#endif

#ifdef WONSY_LOG_TASK_RUNNER
	template< WonSY::Log::LOG_TYPE logType >
	void WonSY::Log::LogManager::AddLog( WonSY::Time::_WsyTime&& time, std::string&& message )
	{
		PushTask( [ logType = logType, time = std::move( time ), message = std::move( message ) ]( WsyLogManagerContextRef key ) mutable
			{
#ifdef WONSY_LOG_WRITE_FILE
				// ���� ������ ������, �α׸� �������� �ʵ��� ó���մϴ�.
				if ( LogManager::GetInstance().Ref( key ).m_fileName.size() ) WONSY_LIKELY
				{
					const auto SetTimeTask = []( WsyLogManagerContextRef key, auto& time )
						{
							LogManager::GetInstance().Ref( key ).m_year  = time.GetYear();
							LogManager::GetInstance().Ref( key ).m_month = time.GetMon();
							LogManager::GetInstance().Ref( key ).m_day   = time.GetDay();
							LogManager::GetInstance().Ref( key ).m_time  = time.GetHour();
						};

					const auto MakeNewFileTask = []( WsyLogManagerContextRef key, auto& time )
						{
							if ( LogManager::GetInstance().Ref( key ).m_writeFilePtr )
								delete LogManager::GetInstance().Ref( key ).m_writeFilePtr;

							LogManager::GetInstance().Ref( key ).m_writeFilePtr = 
								new WsyFileWriter( 
									LogManager::GetInstance().Ref( key ).m_fileName
									+ "_" + std::to_string( time.GetYear() ) 
									/*+ "_"*/ + ( std::to_string( time.GetMon() ).size() == 1 ? ( "0" + std::to_string( time.GetMon() ) ) : std::to_string( time.GetMon() ) )
									/*+ "_"*/ + ( std::to_string( time.GetDay() ).size() == 1 ? ( "0" + std::to_string( time.GetDay() ) ) : std::to_string( time.GetDay() ) )
									+ "_" + ( std::to_string( time.GetHour() ).size() == 1 ? ( "0" + std::to_string( time.GetHour() ) ) : std::to_string( time.GetHour() ) )
									+ ".txt" );
						};

					// ������ ��ȿ���� üũ�մϴ�.
					if ( LogManager::GetInstance().Ref( key ).m_writeFilePtr ) WONSY_LIKELY
					{
						if ( 
							LogManager::GetInstance().Ref( key ).m_year  != time.GetYear() ||
							LogManager::GetInstance().Ref( key ).m_month != time.GetMon()  ||
							LogManager::GetInstance().Ref( key ).m_day   != time.GetDay()  ||
							LogManager::GetInstance().Ref( key ).m_time  != time.GetHour() ) WONSY_UNLIKELY
						{
							SetTimeTask( key, time );
							MakeNewFileTask( key, time );
						}
					}
					else
					{
						SetTimeTask( key, time );
						MakeNewFileTask( key, time );
					}

					LogManager::GetInstance().Ref( key ).m_writeFilePtr->WriteLine( message );
				}
#endif

#ifndef WONSY_UNREAL
#ifdef  WONSY_WINDOW
				WonSY::Log::SetLogColor( logType );
#endif
#endif

				std::cout << message << std::endl;

#ifndef WONSY_UNREAL
#ifdef  WONSY_WINDOW
				WonSY::Log::SetLogColor( WonSY::Log::LOG_TYPE::None );
#endif
#endif
			} );
	}

	WonSY::Log::LogManager::LogManagerContextMemory& WonSY::Log::LogManager::Ref( WsyLogManagerContextRef key )
	{
		return m_lmcm.Ref( key );
	}
#endif

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define ERROR_LOG( logMessage, ... )   if constexpr ( !( LOG_LEVEL >= LOG_LEVEL_NOT_ERROR  ) ){ WonSY::Log::WriteLog< WonSY::Log::LOG_TYPE::LT_ERROR , false >( __FILENAME__, std::source_location::current(), logMessage, ##__VA_ARGS__ ); }
#define WARN_LOG( logMessage, ... )    if constexpr ( !( LOG_LEVEL >= LOG_LEVEL_NOT_WARN   ) ){ WonSY::Log::WriteLog< WonSY::Log::LOG_TYPE::LT_WARN  , false >( __FILENAME__, std::source_location::current(), logMessage, ##__VA_ARGS__ ); }
#define INFO_LOG( logMessage, ... )    if constexpr ( !( LOG_LEVEL >= LOG_LEVEL_NOT_INFO   ) ){ WonSY::Log::WriteLog< WonSY::Log::LOG_TYPE::LT_INFO  , false >( __FILENAME__, std::source_location::current(), logMessage, ##__VA_ARGS__ ); }
#define COMMON_LOG( logMessage, ... )  if constexpr ( !( LOG_LEVEL >= LOG_LEVEL_NOT_COMMON ) ){ WonSY::Log::WriteLog< WonSY::Log::LOG_TYPE::LT_COMMON, false >( __FILENAME__, std::source_location::current(), logMessage, ##__VA_ARGS__ ); }

using WsyLogManager = WonSY::Log::LogManager;

#endif