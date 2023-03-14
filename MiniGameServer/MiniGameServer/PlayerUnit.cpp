#include "pch.h"
#include "PlayerUnit.h"

PlayerUnit::PlayerUnit( const SOCKET& socket )
	: ClientUnit( socket ), m_score(1), m_color(0), m_mp(0), m_skillDuration(0), m_bestScore(0),m_playerState(EPlayerState::NORMAL), m_stunDuration(0)
{
}

PlayerUnit::~PlayerUnit( ) noexcept = default;

void PlayerUnit::Initialize()
{
	//플레이어 초기화
	SetSocket( INVALID_SOCKET );
	SetName( "default" );
	SetScore( 1 );
	SetClientState( EClientState::DISCONNECT );
	SetPlayerState( EPlayerState::NORMAL );
	SetPosition( Position( 0.f, 0.f ) );
	SetRoomNumber( -1 );
	SetId( -1 );
	SetMp( 0 );
	SetSkillDuration( 0 );
	SetStunDuration( 0 );
}