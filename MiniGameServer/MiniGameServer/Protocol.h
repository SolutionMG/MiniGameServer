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
	constexpr float INITPOSITION_X = -1777.f;
	constexpr float INITINTERVAL = 1080.f;
}

namespace InitWord
{
	constexpr float BOX_SIZE = 32.f;
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
	constexpr unsigned char GAMESTART = 3; 
	constexpr unsigned char MOVE = 4;
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
		, type( type )
	{
	}
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
			, owner( owner )
		{}
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
			, owner( owner ), name(),password()
		{}
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
			, owner( owner ), name()
		{}
	};

	// �������� �̴ϰ��� �� ��ȯ ��û �� �ʱ�ȭ ���� ����
	// Ŭ���̾�Ʈ���� �ش� ��Ŷ�� �ް� �÷��̾���� �ʱ� ��ġ, ���� ������ Set
	// owner�� ���� �� �÷��̾���� ������ ��
	struct GameStart
	{
		PacketInfo info;
		int owner; /* �÷��̾� ����, �α��� ���� �� ���� �г������� ����*/
		short color; /* �÷��̾� ���� ���� 0 - red, 1 - blue, 2 - yellow*/
		float x;
		float y;
		// �ʱ� ��ġ �߰��� ����

		GameStart( const int owner )
			: info( sizeof( GameStart ), ServerToClient::GAMESTART )
			, owner( owner ), color(), x(), y( 182.f )
		{}
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
			, owner( owner ), speed(), x(), y(), directionX(), directionY()
		{}
	};
}
#pragma pack(pop)

#endif // !PROTOCOL_H
