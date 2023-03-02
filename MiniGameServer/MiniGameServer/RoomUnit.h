#ifndef ROOMUNIT_H
#define ROOMUNIT_H


class RoomUnit final
{
private:
	std::vector<SOCKET> m_players;
public:
	explicit RoomUnit( ) = default;
	virtual ~RoomUnit( ) = default;

public:
	
	//set
	void PushPlayer( const SOCKET& socket ) { m_players.emplace_back( socket ); }
	void PopPlayer( const SOCKET& socket ) { std::erase_if( m_players, [ &socket ]( SOCKET dest ) { return socket == dest; } );	}

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}

};

#endif // !ROOMUNIT_H
