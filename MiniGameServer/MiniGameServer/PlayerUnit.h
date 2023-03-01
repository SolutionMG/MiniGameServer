#ifndef PLAYERUNIT_H
#define PLAYERUNIT_H


#include "ClientUnit.h"

class RoomUnit;

struct Position
{
	float x;
	float y;
	float z;
};

class PlayerUnit final : public ClientUnit
{

private:
	std::string m_name;
	Position m_position;

public:
	explicit PlayerUnit( const SOCKET& socket );
	virtual ~PlayerUnit( ) noexcept;

	// Set
	void SetName( std::string name );
	void SetPosition( const Position& position ) { memcpy_s( &m_position, sizeof( m_position ), &position, sizeof( position ) ); }


};

#endif // !PLAYERUNIT_H
