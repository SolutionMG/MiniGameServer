#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "MathManager.h"
#include "DataBaseManager.h"
#include "Log.h"

RoomManager::RoomManager()
	:m_updateRoomTimers(), m_timerThread()
{
	m_rooms.reserve( InitServer::MAX_ROOMSIZE );
	m_pushUpdateTimers.reserve( InitServer::MAX_ROOMSIZE );

	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		m_roomNumberPools.push( i );
	}

	for ( int i = 0; i < InitServer::MAX_ROOMSIZE; ++i )
	{
		RoomUnit* room = new RoomUnit;
		room->Initialize();
		m_roomPools.push( room );
	}
}

RoomManager::~RoomManager()
{
	for ( auto& [index, room ] : m_rooms )
	{
		delete room;
		room = nullptr;
	}
	m_rooms.clear();

	RoomUnit* room = nullptr;
	while ( m_roomPools.try_pop( room ) )
	{
		delete room;
		room = nullptr;
	}
}

void RoomManager::Run()
{
	m_taskThread = static_cast< std::jthread >
	( [ this ]( std::stop_token stoken )
	{
		while ( !stoken.stop_requested() )
		{
			WorkTask();
			UpdateRoomTimer();
			std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( InitServer::UPDATE_AWAKE_MS ) );
		}
	} );
}

void RoomManager::UpdateRoomTimer()
{
	// 각 플레이어에 시간 send
	RoomTimer timer;
	while( m_updateRoomTimers.try_pop( timer ))
	{
		const int roomNum = timer.roomNum;
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		RoomUnit* room = m_rooms[ roomNum ];

		if ( !room )
		{
			PRINT_LOG( "room == nullptr" );
			continue;
		}

		if ( timer.requestTime /*요청시간*/ > std::chrono::system_clock::now()/*현재*/ )
		{
			m_pushUpdateTimers.emplace_back( timer );
			//PRINT_LOG( "1초가 지나지 않음" );
			continue;
		}

		unsigned char time = room->GetTime();
		room->SetTime( ++time );

		//std::cout << static_cast<int>(time) << std::endl;

		//각 방의 플레이어들에게 타이머 Send
		auto& players = room->GetPlayers();
		const Packet::Timer timePacket( time );

		for ( auto& index : players )
		{			
			UserManager::GetInstance().PushTask(
			[ index, time, players ]()
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( index );
				if ( !user )
					return;

				const EPlayerState state = user->GetPlayerState();

				if ( state == EPlayerState::STRONGER )
				{
					if ( user->GetSkillDuration() != InitPlayer::SKILLDURATION )
					{
						user->SetSkillDuration( user->GetSkillDuration() + 1 );
						return;
					}
					// 스킬 사용 후 충돌이 일어나지 않아서 스킬 지속시간이 다됐을 경우
					//PRINT_LOG( "스킬 사용 종료" );
					user->SetPlayerState( EPlayerState::NORMAL );
					user->SetSkillDuration( 0 );

					const Packet::SkillEnd skillend( user->GetId() );

					for ( const auto& p : players )
					{
						UserManager::GetInstance().GetUsers()[ p ]->SendPacket( skillend );
					}

				}
				else if ( state == EPlayerState::STUN )
				{
					if ( user->GetStunDuration() != InitPlayer::STUNDURATION )
					{
						user->SetStunDuration( user->GetStunDuration() + 1 );
						return;
					}
					user->SetPlayerState( EPlayerState::NORMAL );
					user->SetStunDuration( 0 );

					const Packet::StunEnd stunend( user->GetId() );

					for ( const auto& p : players )
					{
						UserManager::GetInstance().GetUsers()[ p ]->SendPacket( stunend );
					}
				}
			} );
			
			UserManager::GetInstance().PushTask(
			[ index, timePacket ]()
			{
				UserManager::GetInstance().GetUser( index )->SendPacket( timePacket );
			} );
			
		}

		if ( time == InitWorld::ENDGAMETIME + InitWorld::STARTGAMEDELAY )
		{
			// 게임 종료
			// 타이머에서 더이상 해당 방 갱신 X

			UserManager::GetInstance().PushTask(
			[ players ]()
			{
				Packet::EndGame finalinfo;
				int count = 0;

				for ( const auto index : players )
				{
					PlayerUnit* user = UserManager::GetInstance().GetUser( index );
					if ( !user )
						continue;

					//게임 종료 패킷 전송
					finalinfo.playerInfo[ count ].owner = user->GetId();
					finalinfo.playerInfo[ count++ ].score = user->GetScore();
				}

				for ( const auto index : players )
				{
					//게임 종료 및 결과 패킷 전송
					//봉인
					PlayerUnit* player = UserManager::GetInstance().GetUser( index );
					player->SetClientState( EClientState::GAMEFINISH );
					player->SetPlayerState( EPlayerState::NORMAL );
					player->SendPacket( finalinfo );
					int bestScore = player->GetBestScore();
					int currentScore = player->GetScore();
					if ( bestScore < currentScore )
					{
						player->SetBestScore( currentScore );
						DataBaseManager::GetInstance().BestScoreUpdate( player->GetName(), currentScore );
					}
				}

			} );
			room->SetTime( 0 );
			PRINT_LOG( "게임 종료 시간 도달 - 게임 종료 패킷 전송" );
		}
		else
		{
			//타이머 갱신
			RoomTimer reUpdate( timer.roomNum, std::chrono::system_clock::now() + std::chrono::milliseconds( 1000 - InitServer::UPDATE_AWAKE_MS ) );
			m_pushUpdateTimers.emplace_back( reUpdate );
		}
	}

	for ( const auto& roomTimer : m_pushUpdateTimers )
	{
		m_updateRoomTimers.push( roomTimer );
	}
	m_pushUpdateTimers.clear();
}

void RoomManager::PushRoomNumber( const int number )
{
	m_roomNumberPools.push( number );
}

void RoomManager::PushTimer( const int roomNum )
{
	RoomTimer timer( roomNum, std::chrono::system_clock::now() + std::chrono::milliseconds( 1000 - InitServer::UPDATE_AWAKE_MS ) );
	m_updateRoomTimers.push( timer );
}

void RoomManager::PushRoom( RoomUnit* room )
{
	if ( !room )
		return;
	room->Initialize();
	m_roomPools.push( room );
}

RoomUnit* RoomManager::GetRoomUnitFromPools()
{
	RoomUnit* room = nullptr;
	if ( !m_roomPools.try_pop( room ) )
	{
		room = new RoomUnit;
		room->Initialize();
	}
	return room;
}

RoomUnit* RoomManager::GetRoom( const int index )
{
	if ( m_rooms.find( index ) == m_rooms.end() )
	{
		return nullptr;
	}
	return m_rooms[ index ];
}

const int RoomManager::GetNewRoomNumber( )
{
	int room = -1;
	if ( !m_roomNumberPools.try_pop( room ) )
	{
		room = static_cast< int >( m_roomNumberPools.unsafe_size( ) ) + 1;
	}
	return room;
}

void RoomManager::DeleteRoom( const int index )
{
	m_rooms.erase( index );
}
