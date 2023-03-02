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