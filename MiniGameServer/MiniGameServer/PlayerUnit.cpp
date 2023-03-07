#include "pch.h"
#include "PlayerUnit.h"

PlayerUnit::PlayerUnit( const SOCKET& socket )
	: ClientUnit( socket ), m_score(1), m_color(0)
{
}

PlayerUnit::~PlayerUnit( ) noexcept = default;
