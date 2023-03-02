#ifndef CLIENT_H
#define CLIENT_H

enum class EClientState : char
{
	ACCESS, LOGON, MATCHING, GAME, DISCONNECT /*접속, 로그인, 매칭중, 게임중, 종료 */
};

class ClientUnit
{
protected:
	SOCKET m_socket;
	int m_pId;
	WSAOVERLAPPED_EXTEND m_over;
	EClientState m_state;

	//현재 속해있는 방 번호
	int m_roomNumber; 

	int m_previousReceivePosition;

	std::mutex m_receiveLock;

public:
	explicit ClientUnit( const SOCKET& socket);
	virtual ~ClientUnit( ) noexcept;

	///Set
	void SetSocket( const SOCKET& s )									{ m_socket = s;}
	void SetOverlappedExtend( const WSAOVERLAPPED_EXTEND& over )		{ memcpy_s( &m_over, sizeof( m_over ), &over, sizeof( over ) ); }
	void SetOverlappedOperation( const EOperationType& operation )		{ m_over.type = operation; }
	void SetState( const EClientState& state )							{ m_state = state; }
	void SetPreviousReceivePosition( const int& position )				{ m_previousReceivePosition = position; }
	void SetRoomNumber( const int& roomNum )							{ m_roomNumber = roomNum; }
	void SetId( const int& id )											{ m_pId = id; }

	///Get 
	const SOCKET& GetSocket( )											{ return m_socket; }
	const WSAOVERLAPPED_EXTEND& GetOverlappedExtend( )					{ return m_over; }
	const EClientState& GetState( ) const								{ return m_state; }
	const int& GetPreviousReceivePosition( )							{ return m_previousReceivePosition; }
	const int& GetRoomNum( )											{ return m_roomNumber; }
	const int& GetId()													{ return m_pId; }

	///패킷 송수신 요청 (Overlapped)
	void ReceivePacket( );
	void SendPacket( const char* packet );


};

#endif // !CLIENT_H
