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
	for ( auto& [index, room] : m_rooms )
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

void RoomManager::RunTimer()
{
	//타이머 만들기
	m_timerThread = static_cast< std::jthread >
		( [ this ]( std::stop_token stoken )
		{
			while ( !stoken.stop_requested() )
			{
				UpdateRoomTimer();
				// 1초마다 작동하는 타이머
				std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( 1000 ) );
			}

		} );	
}

void RoomManager::UpdateRoomTimer()
{
	// 각 플레이어에 시간 send
	for(auto& roomNum : m_updateRoomTimers )
	{
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		RoomUnit* room = m_rooms[ roomNum ];
		if ( !room )
		{
			PRINT_LOG( "room == nullptr" );
			m_deleteRoomTimers.emplace_back( roomNum );
			continue;
		}

		unsigned char time = room->GetTime();
		room->SetTime( ++time );

		//각 방의 플레이어들에게 타이머 Send
		auto& players = room->GetPlayers();
		Packet::Timer packet( time );

		for ( auto& index : players )
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser( index );
			if ( !user )
				continue;

			if ( user->GetPlayerState() == EPlayerState::STRONGER )
			{
				UserManager::GetInstance().PushTask(
				[ index, time, roomNum ]()
				{
					PlayerUnit* user = UserManager::GetInstance().GetUser( index );
					if ( !user )
						return;

					if ( user->GetSkillDuration() != InitPlayer::SKILLDURATION )
					{
						user->SetSkillDuration( user->GetSkillDuration() + 1 );
						return;
					}
					// 스킬 사용 후 충돌이 일어나지 않아서 스킬 지속시간이 다됐을 경우
					//PRINT_LOG( "스킬 사용 종료" );
					user->SetPlayerState( EPlayerState::NORMAL);
					user->SetSkillDuration( 0 );

					Packet::SkillEnd skillend( user->GetId() );

					RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
					if ( !room )
					{
						PRINT_LOG( "room == nullptr" );
					}
					auto& players = room->GetPlayers();

					for ( const auto& p : players )
					{
						UserManager::GetInstance().GetUsers()[ p ]->SendPacket( skillend );
					}

				} );
			}

			else if ( user->GetPlayerState() == EPlayerState::STUN )
			{
				UserManager::GetInstance().PushTask(
				[ index, time, roomNum ]()
				{
					PlayerUnit* user = UserManager::GetInstance().GetUser( index );
					if ( !user )
						return;

					if ( user->GetStunDuration() != InitPlayer::STUNDURATION )
					{
						user->SetStunDuration( user->GetStunDuration() + 1 );
						return;
					}
					user->SetPlayerState( EPlayerState::NORMAL );
					user->SetStunDuration( 0 );

					Packet::StunEnd stunend( user->GetId() );

					RoomUnit* room = RoomManager::GetInstance().GetRoom( roomNum );
					if ( !room )
					{
						PRINT_LOG( "room == nullptr" );
					}
					auto& players = room->GetPlayers();

					for ( const auto& p : players )
					{
						UserManager::GetInstance().GetUsers()[ p ]->SendPacket( stunend );
					}
				} );
			}

			UserManager::GetInstance().PushTask(
			[ index, packet ]()
			{
				UserManager::GetInstance().GetUser( index )->SendPacket( packet );
			} );
			
		}

		if ( time == InitWorld::ENDGAMETIME + InitWorld::STARTGAMEDELAY )
		{
			// 게임 종료
			Packet::EndGame finalinfo;
			int count = 0;

			for (const auto& index : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( index );
				if ( !user )
					continue;

				//게임 종료 패킷 전송
				finalinfo.playerInfo[ count ].owner = user->GetId();
				finalinfo.playerInfo[ count++ ].score = user->GetScore();				
			}

			UserManager::GetInstance().PushTask(
			[ finalinfo, players ]()
			{
				for ( const auto& index : players )
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
			PRINT_LOG( "게임 종료 시간 도달 - 게임 종료 패킷 전송" );
		}

		if ( time == InitWorld::ENDGAMETIME + InitWorld::STARTGAMEDELAY + InitWorld::AUTOQUIT )
		{
			// 타이머에서 해당 방 삭제, 게임 종료 패킷 각 플레이어들에게 전송
			room->SetTime( 0 );
			m_deleteRoomTimers.emplace_back( roomNum );
			PRINT_LOG( "게임 오토 종료 도달" );
		}
	}

	// 100초가 넘었을 때 타이머에서 제거 (게임 종료)
	for ( auto& roomNum : m_deleteRoomTimers )
	{
		std::erase_if( m_updateRoomTimers,
			[ roomNum ]( const int& index )
			{ return index == roomNum; });
	}
	m_deleteRoomTimers.clear();
}

void RoomManager::PushRoomNumber( const int& number )
{
	m_roomNumberPools.push( number );
}

void RoomManager::PushTimer( const int& roomNum )
{
	m_updateRoomTimers.emplace_back( roomNum );
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

RoomUnit* RoomManager::GetRoom( const int& index )
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

void RoomManager::DeleteRoom( const int& index )
{
	m_rooms.erase( index );
}
