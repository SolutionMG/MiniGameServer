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
	

	// Get
	virtual const int GetAwakeInterval( ) const noexcept final { return InitServer::UPDATE_AWAKE_MS; };
	virtual const std::string GetName( ) const noexcept final { return  "UserManager"; }
	std::unordered_map<SOCKET, PlayerUnit*>& GetUsers( ) { return m_users; }

private:
	std::unordered_map<SOCKET /*Key*/, PlayerUnit*> m_users;
};

#endif // !USERMANAGER_H
