#include "pch.h"
#include "PlayerUnit.h"

PlayerUnit::PlayerUnit( const SOCKET& socket )
	: ClientUnit( socket )
{
}

PlayerUnit::~PlayerUnit( ) noexcept = default;

void PlayerUnit::SetName( std::string name )
{
}
