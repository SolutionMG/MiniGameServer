#ifndef DEFINE_H
#define DEFINE_H

namespace InitServer
{
	constexpr unsigned short TOTALCORE = 8;
	constexpr unsigned short SERVERPORT = 9000;
	constexpr unsigned short MAX_PLAYERNUM = 99;

	constexpr int UPDATE_AWAKE_MS = 50;

	constexpr int OVERLAPPED_SIZE = 1000;
	constexpr int MAX_ROOMSIZE = 100;

	constexpr float MAX_DISTANCE = 1000.f;
}

//Operation Type
enum class EOperationType : char
{
	RECV, SEND,  ACCEPT, END
};


//Extend Overlapped Structure
struct WSAOVERLAPPED_EXTEND
{
	WSAOVERLAPPED	over;
	WSABUF			wsaBuffer;
	char			networkBuffer[ InitPacket::MAX_BUFFERSIZE ];
	EOperationType	type;
	SOCKET			socket;
};
#endif // !DEFAULT_H
