#ifndef PLAYERUNIT_H
#define PLAYERUNIT_H


#include "ClientUnit.h"

class RoomUnit;

struct Position
{
	float x;
	float y;
public:
	Position() : x( 0.f ), y( 0.f )	{}
	Position( float x, float y ) : x( x ), y( y ) {}
};

class PlayerUnit final : public ClientUnit
{

private:
	std::string m_name;
	Position m_position;
	short m_color;
	unsigned char m_score;

public:
	explicit PlayerUnit( const SOCKET& socket );
	virtual ~PlayerUnit( ) noexcept;

	void Initialize();

	// Get
	const Position& GetPosition( ) { return m_position; }
	const short& GetColor() { return m_color; }
	const unsigned char GetScore() { return m_score;}

	// Set
	void SetName( std::string name ) { m_name = name; }
	void SetPosition( const Position& position ) { memcpy_s( &m_position, sizeof( m_position ), &position, sizeof( position ) ); }
	void SetColor( const short& color ) { m_color = color; }
	void SetScore( const unsigned char& score ) { m_score = score; }
};

#endif // !PLAYERUNIT_H
