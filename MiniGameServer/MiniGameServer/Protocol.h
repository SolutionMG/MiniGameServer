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
}

// PACKET TYPE

// CLIENT
namespace ClientToServer
{
	constexpr short LOGIN_REQUEST = 0;
	constexpr short MOVE = 1;
}
// SERVER
namespace ServerToClient
{
	constexpr short FIRSTINFO = -1;
	constexpr short LOGON_OK = 0;
	constexpr short LOGON_FAILED = 1;
	constexpr short GAMESTART = 2; 
	constexpr short MOVE = 3;
}

// PACKET DECLARE
#pragma pack(push, 1)
struct PacketInfo
{
	/*const*/ unsigned char	 size;
	/*const*/ short type;

public:
	PacketInfo() = default;

	PacketInfo(
		const unsigned char	 size,
		const unsigned short type )
		: size( size )
		, type( type )
	{
	}
};
namespace Packet
{
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

	// �̴ϰ��� �� ��ȯ ��û �� �ʱ�ȭ ���� ����
	struct GameStart
	{
		PacketInfo info;
		int owner; /* �÷��̾� ����, �α��� ���� �� ���� �г������� ����*/
		short color; /* �÷��̾� ���� ���� 0 - red, 1 - blue, 2 - yellow*/
		// �ʱ� ��ġ �߰��� ����

		GameStart( const int owner )
			: info( sizeof( GameStart ), ServerToClient::GAMESTART )
			, owner( owner ), color()
		{}
	};
	
	struct Move
	{
		PacketInfo info;
		int owner;
		float speed;
		float x;
		float y;
		float z;
		float directionX;
		float directionY;
		float directionZ;

		Move( const int owner, const int type/*ClientToServer::Move, ServerToClient::Move*/)
			: info( sizeof( Move ), type )
			, owner( owner ), speed(), x(), y(), z(), directionX(), directionY(), directionZ()
		{}
	};
}
#pragma pack(pop)

#endif // !PROTOCOL_H
