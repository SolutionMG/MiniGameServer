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
	// ���� ���� ����
	std::unordered_map<SOCKET /*Key*/, PlayerUnit*>	m_users;

	// ���� ��ü �޸� Ǯ��
	concurrency::concurrent_queue<PlayerUnit*>		m_userPools;
	concurrency::concurrent_queue<int>				m_pIdPools;

	//Ŀ�ǵ� ����
	std::unordered_map<unsigned char, std::function<void( const SOCKET socket, char* packet )>> m_processFunctions;
public:
	explicit UserManager();
	virtual ~UserManager();

public:

	// Set
	
	// ������ ���� ��ü ����Ǯ ���� ������ ��ȯ
	void PushPlayerUnit( PlayerUnit* player );
	void PushPlayerId( const int id );

	// Get
	virtual const int GetAwakeInterval() const noexcept final	{ return InitServer::UPDATE_AWAKE_MS; };
	virtual const std::string GetName()  const noexcept final	{ return  "UserManager"; }
	
	PlayerUnit* GetUser( const SOCKET socket );
	std::unordered_map<SOCKET, PlayerUnit*>& GetUsers(){ return m_users; }

	//���� ���� ��ü, ����Ǯ���� ��ü ���� ����
	PlayerUnit* GetPlayerUnitFromPools();
	const int GetPlayerId();

	// Packet ó�� ȣ�� �Լ�
	void ProcessPacket( const SOCKET socket, char* packet );

	// ProcessPacket���� ȣ���� ó�� �Լ��� �߰�
	void AddProcess();

	// ��Ŷ Ÿ�Կ����� ó�� �Լ���
	void ProcessLoginRequest( const SOCKET socket, char* packet );
	void ProcessSignupRequest( const SOCKET socket, char* packet );
	void ProcessMove( const SOCKET socket, char* packet );
	void ProcessSkill( const SOCKET socket, char* packet );
	void ProcessMatchingRequest( const SOCKET socket, char* packet );
	void ProcessQuitRoom( const SOCKET socket, char* packet );

	void DeleteUser( const SOCKET socket );
};

#endif // !USERMANAGER_H
