#include "pch.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "RoomUnit.h"
#include "Log.h"
#include "DataBaseManager.h"
#include "RoomManager.h"

UserManager::UserManager( )
	: BaseTaskManager( )
	, m_users ( )
{
	for ( int i = 0; i < InitServer::MAX_PLAYERNUM; ++i )
	{
		PlayerUnit* player = new PlayerUnit(INVALID_SOCKET);
		player->SetName( "Default" );
		player->SetState( EClientState::DISCONNECT );
		player->SetPosition( Position( 0.f, 0.f, 0.f ) );
		player->SetRoomNumber( -1 );
		player->SetId( -1 );
		m_userPools.push( player );
	}

	for ( int i = 0; i < InitServer::MAX_PLAYERNUM; ++i )
	{
		m_pIdPools.push( i );
	}
}

UserManager::~UserManager( )
{
	for ( auto& [id, user] : m_users )
	{
		delete user;
		user = nullptr;
	}
	m_users.clear( );

	PlayerUnit* player = nullptr;
	while ( m_userPools.try_pop( player ) )
	{
		delete player;
		player = nullptr;
	}
}

void UserManager::ProcessPacket( const SOCKET& socket, char* packet )
{
	if ( !packet )
	{
		PRINT_LOG( "packet == nullptr" );
		return;
	}
	switch ( packet[1] )
	{
	case ClientToServer::LOGIN_REQUEST:
	{
		// 로그인 요청 후 
		Packet::LoginRequest data = *reinterpret_cast< Packet::LoginRequest* > ( packet );
		int baseScore = 0;
		if ( DataBaseManager::GetInstance( ).LogOn( data.name, data.password, baseScore ) )
		{
			Packet::LoginResult send(socket, ServerToClient::LOGON_OK);
			strcpy_s( send.name, data.name );
			m_users[ socket ]->SendPacket( send  );
		}
		else
		{
			Packet::LoginResult send( socket, ServerToClient::LOGON_FAILED );
			strcpy_s( send.name, data.name );
			m_users[ socket ]->SendPacket( send );
		}
	}
	break;
	case ClientToServer::MOVE:
	{
		Packet::Move send = *reinterpret_cast< Packet::Move* > ( packet );
		send.info.type = ServerToClient::MOVE;
		//검증필요

		//이동
		int roomNum = m_users[ socket ]->GetRoomNum();
		{
			auto& rooms = RoomManager::GetInstance().GetRooms();

			if ( rooms.find( roomNum ) == rooms.end() )
				return;

			auto& players = rooms[ roomNum ].GetPlayers();

			// 같은 방에 있는 플레이어들에게 정보 전송
			for ( const auto& index : players )
			{
				if ( index == socket )
					continue;

				m_users[ index ]->SendPacket( send );
			}
		}
		
	}
	break;
	default:
	break;
	}
}

void UserManager::PushPlayerUnit( PlayerUnit* player )
{
	if ( !player )
		return;

	player->SetName( "default" );
	player->SetState( EClientState::DISCONNECT );
	player->SetPosition( Position( 0.f, 0.f, 0.f ) );
	player->SetRoomNumber( -1 );
	player->SetId( -1 );

	m_userPools.push( player );
}

void UserManager::PushPlayerId( const int& id )
{
	if ( id < 0 )
		return;
	m_pIdPools.push( id );
}

PlayerUnit* UserManager::GetPlayerUnit( )
{
	PlayerUnit* player = nullptr;
	if ( !m_userPools.try_pop( player ) )
	{
		player = new PlayerUnit(INVALID_SOCKET);
		player->SetName( "default" );
		player->SetState( EClientState::DISCONNECT );
		player->SetPosition( Position( 0.f, 0.f, 0.f ) );
		player->SetRoomNumber( -1 );
		player->SetId( -1 );
	}

	return player;
}

const int& UserManager::GetPlayerId()
{
	// TODO: 여기에 return 문을 삽입합니다.
	int id = -1;
	if ( !m_pIdPools.try_pop( id ) )
	{
		id = m_pIdPools.unsafe_size();
		++id;
	}
	return id;
}
