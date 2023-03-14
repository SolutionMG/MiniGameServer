#ifndef INITDEFINE_H
#define INITDEFINE_H

namespace InitPacket
{
	constexpr unsigned short MAX_BUFFERSIZE = 1024;
	constexpr unsigned short MAX_PACKETSIZE = 255;
}

namespace InitPlayer
{
	constexpr int MAX_NAME = 18;
	constexpr int MAX_PASSWORD = 32;
	constexpr float INITPOSITION_X[ 3 ]		= { 1726.f, 1366.f , 2086.f };
	constexpr float INITPOSITION_Y[ 3 ]		= { 1486.f, 2206.f, 2206.f };
	constexpr float INITDIRECTION_X[ 3 ]	= { 0.f, -0.866f, 0.866f };
	constexpr float INITDIRECTION_Y[ 3 ]	= { -1.f, 0.5f, 0.5f };

	constexpr unsigned char SKILLENABLE		= 15;  /*스킬 사용에 필요한 mp*/
	constexpr unsigned char MPCOUNT			= 1;   /*한번 블록 충돌 시 상승할 mp*/
	constexpr unsigned char SKILLDURATION	= 3;  /*스킬 지속시간*/
	constexpr unsigned char STUNDURATION	= 2;
}

namespace InitWorld
{
	// 블록 갯수 
	constexpr int TILE_COUNTX				= 7;
	constexpr int TILE_COUNTY				= 7;

	// 인게임 플레이어 수
	constexpr int INGAMEPLAYER_NUM			= 3;

	// 블록, 플레이어 크기 정보
	constexpr float TILECOLLIDER_SIZE		= 159.5f;
	constexpr float TILEWITHGAP_SIZE		= 360.f;	/*블록 간 틈을 포함한 거리*/
	constexpr float PLAYERCOLLIDER			= 320.f;		/*플레이어 충돌체 지름*/

	//0번 인덱스 블록 위치
	constexpr float FIRST_TILEPOSITION_X	= 646.f;
	constexpr float FIRST_TILEPOSITION_Y	= 766.f;

	//첫 시작 블록, 색칠되어있을 인덱스
	constexpr int FIRSTTILE_INDEX[ 3 ]		= { 17,30,32 };

	// 게임 종료 시간
	constexpr unsigned char ENDGAMETIME		= 60; /* 실제 게임 시간 = 60초, 게임 시작 후 정지 시간 6초*/
	constexpr unsigned char STARTGAMEDELAY	= 6;

	// 벽 정보
	constexpr float MINIMUM_X = FIRST_TILEPOSITION_X - 25.f;
	constexpr float MINIMUM_Y = FIRST_TILEPOSITION_Y - 25.f;
	constexpr float MAXIMUM_X = FIRST_TILEPOSITION_X + TILEWITHGAP_SIZE * ( TILE_COUNTX - 1 ) + 25.f;
	constexpr float MAXIMUM_Y = FIRST_TILEPOSITION_Y + TILEWITHGAP_SIZE * ( TILE_COUNTY - 1 ) + 25.f;

	constexpr unsigned char LEFT_WALL			= 0;
	constexpr unsigned char RIGHT_WALL			= 1;
	constexpr unsigned char FORWARD_WALL		= 2;
	constexpr unsigned char BACKWARD_WALL		= 3;
	constexpr unsigned char NOTWALLCOLLISION	= 4;

}

// PACKET TYPE
// CLIENT
namespace ClientToServer
{
	constexpr unsigned char LOGIN_REQUEST				= 0;
	constexpr unsigned char MOVE						= 1;
	constexpr unsigned char SKILLUSE_REQUEST			= 2;
	constexpr unsigned char SIGNUP_REQUEST				= 3;
	constexpr unsigned char MATHCING_REQUEST			= 4;
	constexpr unsigned char QUIT_ROOM					= 5;
}
// SERVER
namespace ServerToClient
{
	constexpr unsigned char FIRSTINFO					= 0;
	constexpr unsigned char LOGON_OK					= 1;
	constexpr unsigned char LOGON_FAILED				= 2;
	constexpr unsigned char INITPLAYERS					= 3;
	constexpr unsigned char MOVE						= 4;
	constexpr unsigned char TIME						= 5;
	constexpr unsigned char COLLISION_BLOCK				= 6;
	constexpr unsigned char COLLISION_PLAYER			= 7;
	constexpr unsigned char PLAYERSCORE					= 8;
	constexpr unsigned char COLLISION_WALL				= 9;
	constexpr unsigned char ENDGAME						= 10;
	constexpr unsigned char SKILLUSE_REQUEST_SUCCESS	= 11;
	constexpr unsigned char SKILLUSE_REQUEST_FAILED		= 12;
	constexpr unsigned char MP_UPDATE					= 13;
	constexpr unsigned char SKILLEND					= 14;
	constexpr unsigned char SIGNUP_OK					= 15;
	constexpr unsigned char SIGNUP_FAILED				= 16;
	constexpr unsigned char LOGIN_DUPLICATION			= 17;
	constexpr unsigned char PLAYER_STUNSTART			= 18;
	constexpr unsigned char PLAYER_STUNEND				= 19;

}

#endif // !INITDEFINE
