#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H


// �÷��̾� ����
// �÷��̾ ���� ���� ��û ��, �� ��ȣ �ο�
// �ش� �� ��ȣ�� �ο����� �÷��̾ 3�� ���� �� ���� ���۾����� �̵��϶�� ��Ŷ ����
#include "Singleton.hpp"
#include "BaseTaskManager.h"

class RoomUnit;

struct RoomTimer
{
	int roomNum;
	std::chrono::system_clock::time_point requestTime;

	constexpr bool operator < ( const RoomTimer& other ) const
	{
		return requestTime > other.requestTime;
	}
};

class RoomManager final 
	: public BaseTaskManager
	, public Base::TSingleton< RoomManager >
{
private:
	std::unordered_map<int /*room number*/, RoomUnit*>	m_rooms;

	//�� ��ȣ Ǯ��
	concurrency::concurrent_queue<int>	m_roomNumberPools;
	//�� ��ü Ǯ��
	concurrency::concurrent_queue< RoomUnit* > m_roomPools;


	// �ΰ��� Ÿ�̸Ӹ� ���� ���
	concurrency::concurrent_priority_queue<RoomTimer> m_updateRoomTimers;
	std::vector<RoomTimer> m_pushUpdateTimers;

	std::jthread m_timerThread;

public:
	explicit RoomManager( );
	virtual ~RoomManager();;

	virtual void Run() override;

	// �� Ÿ�̸� ������Ʈ 
	void UpdateRoomTimer();

	// Set
	// �� ��ȣ�� �� ��ȣ Ǯ�� ��ü�� ��ȯ
	void PushRoomNumber(const int number);
	// �ش� ���� Ÿ�̸� ����
	void PushTimer(const int roomNum);
	// �� ��ü Ǯ�� �����̳ʿ� ���� ���� �� ��ȯ
	void PushRoom( RoomUnit* room );

	// Get
	// ������ ����� ���� ��ȯ
	virtual const int GetAwakeInterval() const noexcept final { return InitServer::UPDATE_AWAKE_MS; }
	// ������ �̸� ��ȯ
	virtual const std::string GetName() const noexcept final  { return  "RoomManager"; }	
	// �� ��ü Ǯ�� �����̳ʿ��� �� ��ü ����
	RoomUnit* GetRoomUnitFromPools();
	
	// �� ���� ��ü ��ȯ
	RoomUnit* GetRoom( const int index );

	std::unordered_map<int, RoomUnit*>& GetRooms() { return m_rooms; }
	const int GetNewRoomNumber();

	//�� ����
	void DeleteRoom( const int index );
};

#endif