#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H


// 플레이어 입장
// 플레이어가 게임 시작 요청 시, 방 번호 부여
// 해당 방 번호를 부여받은 플레이어가 3명 존재 시 게임 시작씬으로 이동하라는 패킷 전송
#include "Singleton.hpp"
#include "BaseTaskManager.h"

class RoomUnit;

class RoomManager final 
	: public BaseTaskManager
	, public Base::TSingleton< RoomManager >
{
private:
	std::unordered_map<int /*room number*/, RoomUnit>	m_rooms;
	//방 번호 풀링
	concurrency::concurrent_queue<int>	m_roomPools;

	// 인게임 타이머를 보낼 방들
	std::vector<int/*방 번호*/> m_updateRoomTimers;
	// 인게임에서 삭제할 타이머
	std::vector<int/*방 번호*/> m_deleteRoomTimers;

	std::jthread m_timerThread;

public:
	explicit RoomManager( );
	virtual ~RoomManager( ) = default;

	// 타이머 쓰레드 활성화
	void RunTimer();
	// 방 타이머 업데이트 
	void UpdateRoomTimer();

	// Set
	// 방 번호를 방 번호 풀링 객체에 반환
	void PushRoomNumber(const int& number);
	// 해당 방의 타이머 갱신
	void PushTimer(const int& roomNum);

	// Get
	// 쓰레드 깨우는 간격 반환
	virtual const int GetAwakeInterval() const noexcept final { return InitServer::UPDATE_AWAKE_MS; }
	// 쓰레드 이름 반환
	virtual const std::string GetName() const noexcept final  { return  "RoomManager"; }	
	// 방 관리 객체 반환
	RoomUnit& GetRoom( const int& index );
	std::unordered_map<int, RoomUnit>& GetRooms() { return m_rooms; }
	const int GetNewRoomNumber();

	//방 삭제
	void DeleteRoom( const int& index );
};

#endif