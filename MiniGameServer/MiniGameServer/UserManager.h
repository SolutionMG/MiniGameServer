#ifndef USERMANAGER_H
#define USERMANAGER_H


#include "Singleton.hpp"
#include "BaseTaskManager.h"

class PlayerUnit;

class UserManager final
	: public BaseTaskManager
	, public Base::TSingleton < UserManager >
{
public:
	explicit UserManager( );
	virtual ~UserManager( );

public:

	void ProcessPacket( const SOCKET& socket, char* packet );
	// Set
	
	// ������ ���� ��ü ����Ǯ ���� ������ ��ȯ
	void PushPlayerUnit( PlayerUnit* player );

	// Get
	virtual const int GetAwakeInterval( ) const noexcept final	{ return InitServer::UPDATE_AWAKE_MS; };
	virtual const std::string GetName( ) const noexcept final	{ return  "UserManager"; }
	std::unordered_map<SOCKET, PlayerUnit*>& GetUsers( )		{ return m_users; }

	//���� ���� ��ü, ����Ǯ���� ��ü ���� ����
	PlayerUnit* GetPlayerUnit( );

private:
	// ���� ���� ����
	std::unordered_map<SOCKET /*Key*/, PlayerUnit*> m_users;

	// ���� ��ü �޸� Ǯ��
	concurrency::concurrent_queue<PlayerUnit*> m_userPools; 
};

#endif // !USERMANAGER_H
