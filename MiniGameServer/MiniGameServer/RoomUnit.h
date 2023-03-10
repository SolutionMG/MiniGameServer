#ifndef ROOMUNIT_H
#define ROOMUNIT_H


// 인게임 상의 block 들
struct Tile
{
	int index;
	float x;
	float y;
	short color;
public:
	Tile(): index(-1), x(), y(), color() {}
	Tile(float x, float y) :index(-1),x(x),y(y),color(0) {}
	void operator=( Tile& other ){ index = other.index; x = other.x; y = other.y; color = other.color; }
};

enum class RoomState : char
{
	MATCHING, GAME, END
};

class RoomUnit final
{
private:
	std::vector<SOCKET> m_players;

	//(초단위) 방의 인게임 타임
	unsigned char m_time; 
	
	//방의 발판 블록들
	std::vector<Tile> m_blocks;

	RoomState m_roomState;
public:
	explicit RoomUnit( );
	virtual ~RoomUnit( ) = default;

public:
	// 방 객체 초기화
	void Initialize();

	//set
	void SetTime( const unsigned char& time ){ m_time = time; }
	void PushPlayer( const SOCKET& socket ) { m_players.emplace_back( socket ); }
	void PopPlayer( const SOCKET& socket ) { std::erase_if( m_players, [ &socket ]( SOCKET dest ) { return socket == dest; } );	}
	void SetTileColor( const int& index, const short& color ) { m_blocks[index].color = color; }
	void SetState( const RoomState& state ) { m_roomState = state; }

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}
	const unsigned char& GetTime(){ return m_time; }
	const Tile& GetTile( const int& index ){ return m_blocks[ index ]; }
	const RoomState& GetState() { return m_roomState; }

};

#endif // !ROOMUNIT_H
