#ifndef SERVERMANGER_H
#define SERVERMANGER_H


#include "Singleton.hpp"

class NetworkManager final
	: public Base::TSingleton<NetworkManager>
{
private:
	SOCKET m_listenSocket;
	HANDLE m_iocpHandle;

public:
	explicit NetworkManager( );
	virtual ~NetworkManager( ) noexcept;
	
	// Networking
	bool Initialize( );
	bool Listen( );
	bool RunServer( );
	void ReassemblePacket( char* packet, const DWORD& bytes, const SOCKET& socket );


	// Set

	// Get
	const HANDLE& GetIocpHandle( ) { return m_iocpHandle; }

private:
	bool Accept( WSAOVERLAPPED_EXTEND* over );
	void MainWorkProcess( );
};

#endif // !SERVERMANGER_H
