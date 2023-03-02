#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H


// �÷��̾� ����
// �÷��̾ ���� ���� ��û ��, �� ��ȣ �ο�
// �ش� �� ��ȣ�� �ο����� �÷��̾ 3�� ���� �� ���� ���۾����� �̵��϶�� ��Ŷ ����
#include "Singleton.hpp"
#include "BaseTaskManager.h"

class RoomUnit;

class RoomManager final 
	: public BaseTaskManager
	, public Base::TSingleton< RoomManager >
{
private:
	std::unordered_map<int /*room number*/, RoomUnit>	m_rooms;
	//�� ��ȣ Ǯ��
	concurrency::concurrent_queue<int>	m_roomPools;

public:
	explicit RoomManager( );
	virtual ~RoomManager( ) = default;

public:
	// Set

	// Get
	virtual const int GetAwakeInterval() const noexcept final { return InitServer::UPDATE_AWAKE_MS; }
	virtual const std::string GetName() const noexcept final  { return  "RoomManager"; }	
	std::unordered_map< int, RoomUnit >& GetRooms() { return m_rooms; }
	const int& GetNewRoomNumber();
};

#endif