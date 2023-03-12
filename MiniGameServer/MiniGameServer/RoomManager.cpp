#include "pch.h"
#include "RoomManager.h"
#include "RoomUnit.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "MathManager.h"
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
	//Ÿ�̸� �����
	m_timerThread = static_cast< std::jthread >
		( [ this ]( std::stop_token stoken )
		{
			while ( !stoken.stop_requested() )
			{
				UpdateRoomTimer();
				// 1�ʸ��� �۵��ϴ� Ÿ�̸�
				std::this_thread::sleep_for( static_cast< std::chrono::milliseconds >( 1000 ) );
			}

		} );	
}

void RoomManager::UpdateRoomTimer()
{
	// �� �÷��̾ �ð� send
	for(auto& roomNum : m_updateRoomTimers )
	{
		if ( m_rooms.find( roomNum ) == m_rooms.end() )
			continue;

		RoomUnit* room = m_rooms[ roomNum ];
		if ( !room )
		{
			PRINT_LOG( "room == nullptr" );
			continue;
		}

		unsigned char time = room->GetTime();
		room->SetTime( ++time );

		//�� ���� �÷��̾�鿡�� Ÿ�̸� Send
		auto& players = room->GetPlayers();
		Packet::Timer packet( time );

		for ( auto& index : players )
		{
			PlayerUnit* user = UserManager::GetInstance().GetUser( index );
			if ( !user )
				continue;

			if ( user->GetStronger() )
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
						user->SetStronger( false );
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

			user->SendPacket( packet );
		}

		if ( time == InitWorld::ENDGAMETIME )
		{
			// ���� ����
			// Ÿ�̸ӿ��� �ش� �� ����, ���� ���� ��Ŷ �� �÷��̾�鿡�� ����
			// room->SetTime( 0 );
			// m_deleteRoomTimers.emplace_back( roomNum );

			Packet::EndGame finalinfo;
			int count = 0;

			for ( auto& index : players )
			{
				PlayerUnit* user = UserManager::GetInstance().GetUser( index );
				if ( !user )
					continue;

				//���� ���� ��Ŷ ����
				finalinfo.playerInfo[ count ].owner = user->GetId();
				finalinfo.playerInfo[ count++ ].score = user->GetScore();

				//���� ���� ��Ŷ ����
				//����
				//task�� send?
				//user->SendPacket( finalinfo );
			}

			PRINT_LOG( "���� ���� �ð� ����" );
		}
	}

	// 100�ʰ� �Ѿ��� �� Ÿ�̸ӿ��� ���� (���� ����)
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
