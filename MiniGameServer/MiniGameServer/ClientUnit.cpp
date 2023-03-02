#include "pch.h"
#include "ClientUnit.h"
#include "OverlappedManager.h"

ClientUnit::ClientUnit( const SOCKET& socket )
	: m_socket( socket )
	, m_over()
	, m_state( EClientState::DISCONNECT )
	, m_previousReceivePosition( 0 )
	, m_roomNumber( -1 )
	, m_pId( -1 )
{
	m_over.wsaBuffer.buf = m_over.networkBuffer;
	m_over.wsaBuffer.len = InitPacket::MAX_BUFFERSIZE;
}

ClientUnit::~ClientUnit( ) noexcept = default;

void ClientUnit::ReceivePacket( )
{
	/// Overlapped Receive 요청
	m_receiveLock.lock( );
	ZeroMemory( &m_over, sizeof( m_over ) );
	m_over.wsaBuffer.buf = m_over.networkBuffer + m_previousReceivePosition;
	m_over.wsaBuffer.len = InitPacket::MAX_BUFFERSIZE;
	m_over.type = EOperationType::RECV;
	DWORD flag = 0;
	WSARecv( m_socket, &m_over.wsaBuffer, 1, NULL, &flag, &m_over.over, NULL );
	m_receiveLock.unlock( );
}

void ClientUnit::SendPacket( const char* packet )
{
	/// Overlapped Send 요청
	WSAOVERLAPPED_EXTEND* over = OverlappedManager::GetInstance( ).GetOverlapped( );
	memcpy_s( over->networkBuffer, sizeof( over->networkBuffer ), packet, strlen( packet ) );
	over->wsaBuffer.buf = over->networkBuffer;
	over->wsaBuffer.len = static_cast< decltype( over->wsaBuffer.len ) >( strlen( packet ) );
	WSASend( m_socket, &over->wsaBuffer, 1, 0, 0, &over->over, 0 );
}
