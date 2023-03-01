#include "pch.h"
#include "UserManager.h"
#include "PlayerUnit.h"
#include "Log.h"
#include "Protocol.h"
#include "DataBaseManager.h"

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
		m_userPools.push( player );
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
		Packet::LoginRequest* data = reinterpret_cast< Packet::LoginRequest* > ( packet );
		int baseScore = 0;
		if ( DataBaseManager::GetInstance( ).LogOn( data->name, data->password, baseScore ) )
		{
			Packet::LoginResult send;
			send.info.size = sizeof( Packet::LoginResult );
			send.info.type = ServerToClient::LOGON_OK;
			strcpy_s( send.name, data->name );
			m_users[ socket ]->SendPacket( reinterpret_cast< const char* >( &send ) );
		}
		else
		{
			Packet::LoginResult send;
			send.info.size = sizeof( Packet::LoginResult );
			send.info.type = ServerToClient::LOGON_FAILED;
			strcpy_s( send.name, data->name );
			m_users[ socket ]->SendPacket( reinterpret_cast< const char* >( &send ) );
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
	m_userPools.push( player );
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
	}

	return player;
}
