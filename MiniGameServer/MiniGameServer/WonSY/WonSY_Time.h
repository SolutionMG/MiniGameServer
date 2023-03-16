/*
	Copyright 2021, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once

#ifndef WONSY_TIME
#define WONSY_TIME
#define WONSY_USE_LOCAL_TIME 1

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <ctime>
#include <iomanip>
#include <chrono>

namespace WonSY::Time
{
	using _Time = long long;

	constexpr int MST = -7;
	constexpr int UTC = 0;
	constexpr int CCT = 8;
	constexpr int KST = 9;
	
	enum class INIT_MODE
	{
		ZERO,
		CUR_TIME
	};

	class _WsyTime
	{
		_Time m_timeValue = 0;

	public:
		_WsyTime( const INIT_MODE initMode  = INIT_MODE::ZERO )
			: m_timeValue( initMode == INIT_MODE::ZERO ? 0 : std::time( nullptr ) )
		{
		}

		_WsyTime( const _Time timeValue )
			: m_timeValue( timeValue )
		{
		}

		//_WsyTime( std::string timeString );

		~_WsyTime() = default;
	
		std::string ToString() const
		{
			const auto localTime = *std::localtime( &m_timeValue );

			std::ostringstream os;
			os << std::put_time( &localTime, "%y-%m-%d %H:%M:%S" );

			return os.str();

			//return 
			//	std::to_string( GetYear() ) + "-" +
			//	std::to_string( GetMon() ) + "-" +
			//	std::to_string( GetDay() ) + " " +
			//	std::to_string( GetHour() ) + ":" +
			//	std::to_string( GetMin() ) + ":" +
			//	std::to_string( GetSec() );
		}

		_Time GetTimeValue() const
		{
			return m_timeValue;
		}

#if WONSY_USE_LOCAL_TIME == 1
		int GetYear(){ tm* tm = std::localtime( &m_timeValue ); return tm->tm_year + 1900; }
		int GetMon() { tm* tm = std::localtime( &m_timeValue ); return tm->tm_mon + 1;     }
		int GetDay() { tm* tm = std::localtime( &m_timeValue ); return tm->tm_mday;        }
		int GetHour(){ tm* tm = std::localtime( &m_timeValue ); return tm->tm_hour;        }
		int GetMin() { tm* tm = std::localtime( &m_timeValue ); return tm->tm_min;         }
		int GetSec() { tm* tm = std::localtime( &m_timeValue ); return tm->tm_sec;         }
#else
		int GetYear(){ tm* tm = std::gmtime( &m_timeValue ); return tm->tm_year + 1900;         }
		int GetMon() { tm* tm = std::gmtime( &m_timeValue ); return tm->tm_mon;                 }
		int GetDay() { tm* tm = std::gmtime( &m_timeValue ); return tm->tm_mday;                }
		int GetHour(){ tm* tm = std::gmtime( &m_timeValue ); return ( tm->tm_hour + KST ) % 24; }
		int GetMin() { tm* tm = std::gmtime( &m_timeValue ); return tm->tm_hour;                }
		int GetSec() { tm* tm = std::gmtime( &m_timeValue ); return tm->tm_sec;                 }
#endif

	public:
		bool operator==( _WsyTime& time )
		{
			return m_timeValue == time.m_timeValue;
		}

		_WsyTime operator+( const _WsyTime& time ) const
		{
			return _WsyTime( m_timeValue + time.m_timeValue );
		}

		_WsyTime operator-( const _WsyTime& time ) const
		{
			return _WsyTime( m_timeValue - time.m_timeValue );
		}

		friend std::ostream& operator<<( std::ostream& out, const WonSY::Time::_WsyTime& time )
		{
			out << time.ToString();
			return out;
		}
	};

#pragma region [ static function ]

	_WsyTime GetCurTime();

	_Time GetCurTimeValue();

	_Time GetMilliseconds() noexcept;

#pragma endregion
}

using WsyTime     = WonSY::Time::_WsyTime;

namespace WsyTimeUtil = WonSY::Time;

#endif
