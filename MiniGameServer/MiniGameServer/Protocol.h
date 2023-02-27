#ifndef PROTOCOL_H
#define PROTOCOL_H

namespace InitPlayer
{
	constexpr int MAX_NAME = 16;
	constexpr int MAX_PASSWORD = 32;
}

// PACKET TYPE

// CLIENT
namespace ClientToServer
{
	constexpr int LOGIN_REQUEST = 0;
}
// SERVER
namespace ServerToClient
{
	constexpr int LOGON_OK = 0;
	constexpr int LOGON_FAILED = 1;
}

// PACKET DECLARE
#pragma pack(push, 1)
struct PacketInfo
{
	unsigned char	size;
	unsigned short	type;
};
namespace Packet
{
	struct LoginRequest
	{
		PacketInfo packetinfo;
		char name[ InitPlayer::MAX_NAME ];
		char password[ InitPlayer::MAX_PASSWORD ];
	};

	struct LoginResult
	{
		PacketInfo packetInfo;
		char name[ InitPlayer::MAX_NAME ];
	};
}
#pragma pack(pop)

#endif // !PROTOCOL_H
