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
};

// 인게임 상의 item들
struct Item
{
	int index;
	float x;
	float y;
	unsigned char itemType;
public:
	Item() : index(), x(), y(), itemType() {}
	Item( float x, float y, unsigned char itemType, int index ) : x( x ), y( y ), itemType( itemType ), index(index) {}
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

	//맵상에 배치된 아이템들
	std::vector<Item> m_items;

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

	// 아이템 관리 객체에 해당 방에 젠 되어있는 아이템 정보 넣기
	void PushItem( const Item& item ) { m_items.emplace_back( item ); }

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}
	const unsigned char& GetTime(){ return m_time; }
	const Tile& GetTile( const int& index ){ return m_blocks[ index ]; }
	const std::vector<Item>& GetItems() { return m_items; }

	// 플레이어와 충돌한 아이템 삭제
	void PopItem( const int& index ) { std::erase_if( m_items, [ &index ]( const Item& dest ) {return index == dest.index; } ); }

};

#endif // !ROOMUNIT_H
