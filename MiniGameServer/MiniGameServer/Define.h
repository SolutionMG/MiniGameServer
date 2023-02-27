#ifndef DEFINE_H
#define DEFINE_H

namespace InitServer
{
	constexpr unsigned short TOTALCORE = 8;
	constexpr unsigned short SERVERPORT = 9000;
	constexpr unsigned short MAX_BUFFERSIZE = 1024;
	constexpr unsigned short MAX_PACKETSIZE = 255;
	constexpr unsigned short MAX_PLAYERNUM = 99;

	constexpr int UPDATE_AWAKE_MS = 100;

	constexpr int OVERLAPPED_SIZE = 1000;
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
	char			networkBuffer[ InitServer::MAX_BUFFERSIZE ];
	EOperationType	type;
	SOCKET			socket;
};
#endif // !DEFAULT_H
