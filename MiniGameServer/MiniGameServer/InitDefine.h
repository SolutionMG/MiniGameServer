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

	constexpr unsigned char SKILLENABLE		= 15;  /*��ų ��뿡 �ʿ��� mp*/
	constexpr unsigned char MPCOUNT			= 1;   /*�ѹ� ��� �浹 �� ����� mp*/
	constexpr unsigned char SKILLDURATION	= 3;  /*��ų ���ӽð�*/
	constexpr unsigned char STUNDURATION	= 2;
}

namespace InitWorld
{
	// ��� ���� 
	constexpr int TILE_COUNTX				= 7;
	constexpr int TILE_COUNTY				= 7;

	// �ΰ��� �÷��̾� ��
	constexpr int INGAMEPLAYER_NUM			= 3;

	// ���, �÷��̾� ũ�� ����
	constexpr float TILECOLLIDER_SIZE		= 159.5f;
	constexpr float TILEWITHGAP_SIZE		= 360.f;	/*��� �� ƴ�� ������ �Ÿ�*/
	constexpr float PLAYERCOLLIDER			= 320.f;		/*�÷��̾� �浹ü ����*/

	//0�� �ε��� ��� ��ġ
	constexpr float FIRST_TILEPOSITION_X	= 646.f;
	constexpr float FIRST_TILEPOSITION_Y	= 766.f;

	//ù ���� ���, ��ĥ�Ǿ����� �ε���
	constexpr int FIRSTTILE_INDEX[ 3 ]		= { 17,30,32 };

	// ���� ���� �ð�
	constexpr unsigned char ENDGAMETIME		= 60; /* ���� ���� �ð� = 60��, ���� ���� �� ���� �ð� 6��*/
	constexpr unsigned char STARTGAMEDELAY	= 6;

	// �� ����
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
