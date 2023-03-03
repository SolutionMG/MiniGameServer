#ifndef ROOMUNIT_H
#define ROOMUNIT_H


class RoomUnit final
{
private:
	std::vector<SOCKET> m_players;
	unsigned char m_time; //√ ¥‹¿ß
public:
	explicit RoomUnit( );
	virtual ~RoomUnit( ) = default;

public:
	
	//set
	void SetTime( const unsigned char& time ){ m_time = time; }
	void PushPlayer( const SOCKET& socket ) { m_players.emplace_back( socket ); }
	void PopPlayer( const SOCKET& socket ) { std::erase_if( m_players, [ &socket ]( SOCKET dest ) { return socket == dest; } );	}

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}
	const unsigned char& GetTime(){ return m_time; }

};

#endif // !ROOMUNIT_H
