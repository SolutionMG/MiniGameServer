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

	void SetName( std::string name );



};

#endif // !PLAYERUNIT_H
