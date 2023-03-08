#ifndef ROOMUNIT_H
#define ROOMUNIT_H


// �ΰ��� ���� block ��
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

// �ΰ��� ���� item��
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

	//(�ʴ���) ���� �ΰ��� Ÿ��
	unsigned char m_time; 
	
	//���� ���� ��ϵ�
	std::vector<Tile> m_blocks;

	//�ʻ� ��ġ�� �����۵�
	std::vector<Item> m_items;

	RoomState m_roomState;
public:
	explicit RoomUnit( );
	virtual ~RoomUnit( ) = default;

public:
	// �� ��ü �ʱ�ȭ
	void Initialize();

	//set
	void SetTime( const unsigned char& time ){ m_time = time; }
	void PushPlayer( const SOCKET& socket ) { m_players.emplace_back( socket ); }
	void PopPlayer( const SOCKET& socket ) { std::erase_if( m_players, [ &socket ]( SOCKET dest ) { return socket == dest; } );	}
	void SetTileColor( const int& index, const short& color ) { m_blocks[index].color = color; }

	// ������ ���� ��ü�� �ش� �濡 �� �Ǿ��ִ� ������ ���� �ֱ�
	void PushItem( const Item& item ) { m_items.emplace_back( item ); }

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}
	const unsigned char& GetTime(){ return m_time; }
	const Tile& GetTile( const int& index ){ return m_blocks[ index ]; }
	const std::vector<Item>& GetItems() { return m_items; }

	// �÷��̾�� �浹�� ������ ����
	void PopItem( const int& index ) { std::erase_if( m_items, [ &index ]( const Item& dest ) {return index == dest.index; } ); }

};

#endif // !ROOMUNIT_H
