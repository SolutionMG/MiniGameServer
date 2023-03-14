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
	// �� �÷��̾ �ð� send
	RoomTimer timer;
	while( m_updateRoomTimers.try_pop( timer ))
	{
		int roomNum = timer.roomNum;
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		RoomUnit* room = m_rooms[ roomNum ];

		if ( !room )
		{
			PRINT_LOG( "room == nullptr" );
			continue;
		}

		if ( timer.requestTime /*��û�ð�*/ > std::chrono::system_clock::now()/*����*/ )
		{
			m_pushUpdateTimers.emplace_back( timer );
			PRINT_LOG( "1�ʰ� ������ ����" );
			continue;
		}

		unsigned char time = room->GetTime();
		room->SetTime( ++time );

		std::cout << static_cast<int>(time) << std::endl;

		//�� ���� �÷��̾�鿡�� Ÿ�̸� Send
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
					// ��ų ��� �� �浹�� �Ͼ�� �ʾƼ� ��ų ���ӽð��� �ٵ��� ���
					//PRINT_LOG( "��ų ��� ����" );
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
			// ���� ����
			Packet::EndGame finalinfo;
			int count = 0;

			for (const auto& index : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( index );
				if ( !user )
					continue;

				//���� ���� ��Ŷ ����
				finalinfo.playerInfo[ count ].owner = user->GetId();
				finalinfo.playerInfo[ count++ ].score = user->GetScore();				
			}

			UserManager::GetInstance().PushTask(
			[ finalinfo, players ]()
			{
				for ( const auto& index : players )
				{
					//���� ���� �� ��� ��Ŷ ����
					//����
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
			PRINT_LOG( "���� ���� �ð� ���� - ���� ���� ��Ŷ ����" );
		}

		if ( time == InitWorld::ENDGAMETIME + InitWorld::STARTGAMEDELAY + InitWorld::AUTOQUIT )
		{
			// Ÿ�̸ӿ��� ���̻� �ش� �� ���� X
			room->SetTime( 0 );
			PRINT_LOG( "���� ���� ���� ����" );
		}
		else
		{
			//Ÿ�̸� ����
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

void RoomManager::PushRoomNumber( const int& number )
{
	m_roomNumberPools.push( number );
}

void RoomManager::PushTimer( const int& roomNum )
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
