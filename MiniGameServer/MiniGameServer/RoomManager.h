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

	// �ΰ��� Ÿ�̸Ӹ� ���� ���
	std::vector<int/*�� ��ȣ*/> m_updateRoomTimers;
	// �ΰ��ӿ��� ������ Ÿ�̸�
	std::vector<int/*�� ��ȣ*/> m_deleteRoomTimers;

	std::jthread m_timerThread;

public:
	explicit RoomManager( );
	virtual ~RoomManager( ) = default;

	// Ÿ�̸� ������ Ȱ��ȭ
	void RunTimer();
	// �� Ÿ�̸� ������Ʈ 
	void UpdateRoomTimer();

	// Set
	// �� ��ȣ�� �� ��ȣ Ǯ�� ��ü�� ��ȯ
	void PushRoomNumber(const int& number);
	// �ش� ���� Ÿ�̸� ����
	void PushTimer(const int& roomNum);

	// Get
	// ������ ����� ���� ��ȯ
	virtual const int GetAwakeInterval() const noexcept final { return InitServer::UPDATE_AWAKE_MS; }
	// ������ �̸� ��ȯ
	virtual const std::string GetName() const noexcept final  { return  "RoomManager"; }	
	// �� ���� ��ü ��ȯ
	RoomUnit& GetRoom( const int& index );
	std::unordered_map<int, RoomUnit>& GetRooms() { return m_rooms; }
	const int GetNewRoomNumber();

	//�� ����
	void DeleteRoom( const int& index );
};

#endif