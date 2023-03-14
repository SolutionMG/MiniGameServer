#ifndef USERMANAGER_H
#define USERMANAGER_H


#include "Singleton.hpp"
#include "BaseTaskManager.h"

#include <functional>

class PlayerUnit;

class UserManager final
	: public BaseTaskManager
	, public Base::TSingleton < UserManager >
{
private:
	// 접속 유저 관리
	std::unordered_map<SOCKET /*Key*/, PlayerUnit*>	m_users;

	// 유저 객체 메모리 풀링
	concurrency::concurrent_queue<PlayerUnit*>		m_userPools;
	concurrency::concurrent_queue<int>				m_pIdPools;

	//커맨드 접근
	std::unordered_map<unsigned char, std::function<void( const SOCKET socket, char* packet )>> m_processFunctions;
public:
	explicit UserManager();
	virtual ~UserManager();

public:

	// Set
	
	// 종료한 유저 객체 유저풀 관리 변수에 반환
	void PushPlayerUnit( PlayerUnit* player );
	void PushPlayerId( const int id );

	// Get
	virtual const int GetAwakeInterval() const noexcept final	{ return InitServer::UPDATE_AWAKE_MS; };
	virtual const std::string GetName()  const noexcept final	{ return  "UserManager"; }
	
	PlayerUnit* GetUser( const SOCKET socket );
	std::unordered_map<SOCKET, PlayerUnit*>& GetUsers(){ return m_users; }

	//접속 유저 객체, 유저풀에서 객체 꺼내 전달
	PlayerUnit* GetPlayerUnitFromPools();
	const int GetPlayerId();

	// Packet 처리 호출 함수
	void ProcessPacket( const SOCKET socket, char* packet );

	// ProcessPacket에서 호출할 처리 함수들 추가
	void AddProcess();

	// 패킷 타입에따른 처리 함수들
	void ProcessLoginRequest( const SOCKET socket, char* packet );
	void ProcessSignupRequest( const SOCKET socket, char* packet );
	void ProcessMove( const SOCKET socket, char* packet );
	void ProcessSkill( const SOCKET socket, char* packet );
	void ProcessMatchingRequest( const SOCKET socket, char* packet );
	void ProcessQuitRoom( const SOCKET socket, char* packet );

	void DeleteUser( const SOCKET socket );
};

#endif // !USERMANAGER_H
