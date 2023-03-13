#include "pch.h"
#include "PlayerUnit.h"

PlayerUnit::PlayerUnit( const SOCKET& socket )
	: ClientUnit( socket ), m_score(1), m_color(0), m_stronger(false), m_mp(0), m_skillDuration(0), m_bestScore(0)
{
}

PlayerUnit::~PlayerUnit( ) noexcept = default;

void PlayerUnit::Initialize()
{
	//플레이어 초기화
	SetSocket( INVALID_SOCKET );
	SetName( "default" );
	SetScore( 1 );
	SetState( EClientState::DISCONNECT );
	SetPosition( Position( 0.f, 0.f ) );
	SetRoomNumber( -1 );
	SetId( -1 );
	SetMp( 0 );
	SetStronger( false );
	SetSkillDuration( 0 );
}