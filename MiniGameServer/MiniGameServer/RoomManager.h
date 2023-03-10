#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H


// 플레이어 입장
// 플레이어가 게임 시작 요청 시, 방 번호 부여
// 해당 방 번호를 부여받은 플레이어가 3명 존재 시 게임 시작씬으로 이동하라는 패킷 전송
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

	//방 번호 풀링
	concurrency::concurrent_queue<int>	m_roomNumberPools;
	//방 객체 풀링
	concurrency::concurrent_queue< RoomUnit* > m_roomPools;


	// 인게임 타이머를 보낼 방들
	concurrency::concurrent_priority_queue<RoomTimer> m_updateRoomTimers;
	std::vector<RoomTimer> m_pushUpdateTimers;

	std::jthread m_timerThread;

public:
	explicit RoomManager( );
	virtual ~RoomManager();;

	virtual void Run() override;

	// 방 타이머 업데이트 
	void UpdateRoomTimer();

	// Set
	// 방 번호를 방 번호 풀링 객체에 반환
	void PushRoomNumber(const int number);
	// 해당 방의 타이머 갱신
	void PushTimer(const int roomNum);
	// 방 객체 풀링 컨테이너에 게임 종료 방 반환
	void PushRoom( RoomUnit* room );

	// Get
	// 쓰레드 깨우는 간격 반환
	virtual const int GetAwakeInterval() const noexcept final { return InitServer::UPDATE_AWAKE_MS; }
	// 쓰레드 이름 반환
	virtual const std::string GetName() const noexcept final  { return  "RoomManager"; }	
	// 방 객체 풀링 컨테이너에서 방 객체 추출
	RoomUnit* GetRoomUnitFromPools();
	
	// 방 관리 객체 반환
	RoomUnit* GetRoom( const int index );

	std::unordered_map<int, RoomUnit*>& GetRooms() { return m_rooms; }
	const int GetNewRoomNumber();

	//방 삭제
	void DeleteRoom( const int index );
};

#endif