#ifndef PROTOCOL_H
#define PROTOCOL_H

namespace InitPacket
{
	constexpr unsigned short MAX_BUFFERSIZE = 1024;
	constexpr unsigned short MAX_PACKETSIZE = 255;
}

namespace InitPlayer
{
	constexpr int MAX_NAME = 16;
	constexpr int MAX_PASSWORD = 32;
	constexpr float INITPOSITION_X[ 3 ] = {1726.f, 1366.f , 2086.f};
	constexpr float INITPOSITION_Y[ 3 ] = {1486.f, 2206.f, 2206.f};
	constexpr float INITDIRECTION_X[ 3 ] = {0.f, -0.5f, -0.5f};
	constexpr float INITDIRECTION_Y[ 3 ] = { 0.f,-0.866025f,0.866025f };
}

namespace InitWorld
{
	// ��� ����
	constexpr int TILE_COUNTX = 7;
	constexpr int TILE_COUNTY = 7;

	// ������
	constexpr float TILECOLLIDER_SIZE = 159.5f;
	constexpr float TILEWITHGAP_SIZE = 360.f; /*��� �� ƴ�� ������ �Ÿ�*/

	//0�� �ε��� ��� ��ġ
	constexpr float FIRST_TILEPOSITION_X = 646.f;
	constexpr float FIRST_TILEPOSITION_Y = 766.f;

	//ù ���� ��� ��ĥ �ε���
	constexpr int FIRSTTILE_COLOR[ 3 ] = { 17,30,32 };
}
// PACKET TYPE

// CLIENT
namespace ClientToServer
{
	constexpr unsigned char LOGIN_REQUEST = 0;
	constexpr unsigned char MOVE = 1;
}
// SERVER
namespace ServerToClient
{
	constexpr unsigned char FIRSTINFO = 0;
	constexpr unsigned char LOGON_OK = 1;
	constexpr unsigned char LOGON_FAILED = 2;
	constexpr unsigned char INITPLAYERS = 3; 
	constexpr unsigned char MOVE = 4;
	constexpr unsigned char TIME = 5;
	constexpr unsigned char COLLISION_BLOCK = 6;
	constexpr unsigned char PLAYERSCORE = 7;
}

// PACKET DECLARE
#pragma pack(push, 1)
struct PacketInfo
{
	/*const*/ unsigned char	size;
	/*const*/ unsigned char type;

public:
	PacketInfo() = default;

	PacketInfo(
		const unsigned char	size,
		const unsigned char type )
		: size( size )
		, type( type ) {}
};
namespace Packet
{
	// Ŭ���̾�Ʈ ù ���� �� ������ ���� �ε��� ����
	// Ŭ���̾�Ʈ������ �ش� ��Ŷ�� �ް� owner�� ������ ������ �÷��̾ Set
	struct FirstPlayer
	{
		//const PacketInfo info;
		PacketInfo info;
		int owner;

		FirstPlayer(const int owner)
			: info( sizeof( FirstPlayer ), ServerToClient::FIRSTINFO )
			, owner( owner ) {}
	};

	// �α��� ��û
	// Ŭ���̾�Ʈ������ �ش� ��Ŷ�� ���� �α��� ��û�� ����
	struct LoginRequest
	{
		PacketInfo info;
		char name[ InitPlayer::MAX_NAME ];
		char password[ InitPlayer::MAX_PASSWORD ];
		int owner;

		LoginRequest( const int owner )
			: info( sizeof( LoginRequest ), ClientToServer::LOGIN_REQUEST )
			, owner( owner ), name(),password() {}
	};
	// �α��� ���
	// �������� �α��� ����� Ŭ���̾�Ʈ���� ���� -> ��Ŷ Ÿ���� Login Failed, LoginOk �� �ϳ�
	// ��Ŷ Ÿ��(info.type)�� LoginOk�� �г��� Ŭ���̾�Ʈ ������ �÷��̾ Set
	// LoginFailed�� �α��� ���� ���� ���.
	struct LoginResult
	{
		PacketInfo info;
		char name[ InitPlayer::MAX_NAME ];
		int owner;

		// �·��� �߰��� �� ����
		LoginResult( const int owner, const int type/*Login Failed, Login Ok*/)
			: info( sizeof( LoginResult ), type )
			, owner( owner ), name() {}
	};

	// �������� �̴ϰ��� �� ��ȯ ��û �� �ʱ�ȭ ���� ����
	// Ŭ���̾�Ʈ���� �ش� ��Ŷ�� �ް� �÷��̾���� �ʱ� ��ġ, ���� ������ Set
	// owner�� ���� �� �÷��̾���� ������ ��
	struct InitPlayers
	{
		PacketInfo info;
		int owner;			/* �÷��̾� ����, �α��� ���� �� ���� �г������� ����*/
		short color;		/* �÷��̾� ���� ���� 0 - red, 1 - blue, 2 - yellow*/
		float x;			/*x ��ǥ*/
		float y;			/*y ��ǥ*/
		float directionX;	/*X���� ����*/
		float directionY;	/*Y���� ����*/

		// �ʱ� ��ġ �߰��� ����

		InitPlayers( const int owner )
			: info( sizeof( InitPlayers ), ServerToClient::INITPLAYERS )
			, owner( owner ), color(), x(), y(), directionX(), directionY() {}
	};
	
	// Ŭ���̾�Ʈ�� �̵� ��Ŷ �������� ����
	// �������� ��ġ ���� �� �ٸ� Ŭ���̾�Ʈ �鿡�� ����
	// Ŭ���̾�Ʈ�� �ش� ��Ŷ�� ���� ��� owner�� ���� � �÷��̾��� �������� Ȯ�� ��
	// �ش� �÷��̾��� ��ġ, ����, �ӵ��� �����Ͽ� ����
	struct Move
	{
		PacketInfo info;
		int owner;
		float speed;
		float x;
		float y;
		float directionX;
		float directionY;

		Move( const int owner, const int type/*ClientToServer::Move, ServerToClient::Move*/)
			: info( sizeof( Move ), type )
			, owner( owner ), speed(), x(), y(), directionX(), directionY()	{}
	};

	// �÷��̾�� ��ϰ��� �浹 �߻� �� Ŭ�󿡰� ��Ŷ ����
	// 
	struct CollisionTile
	{
		PacketInfo info;
		int owner;
		int tileIndex;

		CollisionTile(const int owner, const int tileIndex)
			:info(sizeof( CollisionTile ), ServerToClient::COLLISION_BLOCK ),owner(owner), tileIndex( tileIndex ) {}
	};


	// ������ Ŭ���̾�Ʈ���� ������ �ΰ��� Ÿ�̸�
	// ����: 1��
	struct Timer
	{
		PacketInfo info;
		unsigned char time; /*�� ����*/
		Timer(unsigned char time)
			:info(sizeof(Timer), ServerToClient::TIME), time(time) {}
	};

	// �÷��̾� ���� ��Ŷ
	struct Score
	{
		PacketInfo info;
		int owner;
		unsigned char score;
		Score(const int owner, unsigned char score ) :info(sizeof( Score ), ServerToClient::PLAYERSCORE ), owner(owner), score(score){}
	};
}
#pragma pack(pop)

#endif // !PROTOCOL_H
