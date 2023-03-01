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
	
	// 종료한 유저 객체 유저풀 관리 변수에 반환
	void PushPlayerUnit( PlayerUnit* player );

	// Get
	virtual const int GetAwakeInterval( ) const noexcept final	{ return InitServer::UPDATE_AWAKE_MS; };
	virtual const std::string GetName( ) const noexcept final	{ return  "UserManager"; }
	std::unordered_map<SOCKET, PlayerUnit*>& GetUsers( )		{ return m_users; }

	//접속 유저 객체, 유저풀에서 객체 꺼내 전달
	PlayerUnit* GetPlayerUnit( );

private:
	// 접속 유저 관리
	std::unordered_map<SOCKET /*Key*/, PlayerUnit*> m_users;

	// 유저 객체 메모리 풀링
	concurrency::concurrent_queue<PlayerUnit*> m_userPools; 
};

#endif // !USERMANAGER_H
