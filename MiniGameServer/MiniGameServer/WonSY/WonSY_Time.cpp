/*
	Copyright 2021, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#include "WonSY_Time.h"

//#include "WonSY_String.h"

WonSY::Time::_WsyTime WonSY::Time::GetCurTime()
{
	return _WsyTime( INIT_MODE::CUR_TIME );
}

WonSY::Time::_Time WonSY::Time::GetCurTimeValue()
{
	return _WsyTime( INIT_MODE::CUR_TIME ).GetTimeValue();
}

WonSY::Time::_Time WonSY::Time::GetMilliseconds() noexcept
{
	// 1000���� ���� ���� �ʿ��ҋ�
	//return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count() % 1000;

	// ��ü ���� �ʿ��Ҷ�
	return std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}
