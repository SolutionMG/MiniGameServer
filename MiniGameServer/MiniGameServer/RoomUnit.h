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
	float x;
	float y;
	unsigned char itemType;
public:
	Item() : x(), y(), itemType() {}
	Item( float x, float y, unsigned char itemType ) : x( x ), y( y ), itemType( itemType ) {}
};

class RoomUnit final
{
private:
	std::vector<SOCKET> m_players;

	//(�ʴ���) ���� �ΰ��� Ÿ��
	unsigned char m_time; 
	
	//���� ���� ��ϵ�
	std::vector<Tile> m_blocks;
public:
	explicit RoomUnit( );
	virtual ~RoomUnit( ) = default;

public:
	
	//set
	void SetTime( const unsigned char& time ){ m_time = time; }
	void PushPlayer( const SOCKET& socket ) { m_players.emplace_back( socket ); }
	void PopPlayer( const SOCKET& socket ) { std::erase_if( m_players, [ &socket ]( SOCKET dest ) { return socket == dest; } );	}
	void SetTileColor( const int& index, const short& color ) { m_blocks[index].color = color; }

	//get
	const std::vector<SOCKET>& GetPlayers( ) {return m_players;}
	const unsigned char& GetTime(){ return m_time; }
	const Tile& GetTile( const int& index ){ return m_blocks[ index ]; }

};

#endif // !ROOMUNIT_H
