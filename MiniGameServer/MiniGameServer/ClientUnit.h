#ifndef CLIENT_H
#define CLIENT_H

enum class EClientState : char
{
	ACCESS, LOGON, GAME, END
};

class ClientUnit
{
protected:
	SOCKET m_socket;
	WSAOVERLAPPED_EXTEND m_over;
	EClientState m_state;

	unsigned char m_previousReceivePosition;

	std::mutex m_receiveLock;

public:
	explicit ClientUnit( const SOCKET& socket);
	virtual ~ClientUnit( ) noexcept;

	///Set
	void SetSocket( const SOCKET& s )									{ m_socket = s;}
	void SetOverlappedExtend( const WSAOVERLAPPED_EXTEND& over )		{ memcpy_s( &m_over, sizeof( m_over ), &over, sizeof( over ) ); }
	void SetOverlappedOperation( const EOperationType& operation )		{ m_over.type = operation; }
	void SetState( const EClientState& state )							{ m_state = state; }
	void SetPreviousReceivePosition( const unsigned char& position )	{ m_previousReceivePosition = position; }

	///Get 
	const SOCKET& GetSocket( )											{ return m_socket; }
	const WSAOVERLAPPED_EXTEND& GetOverlappedExtend( )					{ return m_over; }
	const EClientState& GetState( ) const								{ return m_state; }
	const unsigned char& GetPreviousReceivePosition( )					{ return m_previousReceivePosition; }

	///패킷 송수신 요청 (Overlapped)
	void ReceivePacket( );
	void SendPacket( const char* packet );


};

#endif // !CLIENT_H
