#ifndef INITDEFINE_H
#define INITDEFINE_H

namespace InitPacket
{
	constexpr unsigned short MAX_BUFFERSIZE = 1024;
	constexpr unsigned short MAX_PACKETSIZE = 255;
}

namespace InitPlayer
{
	constexpr int MAX_NAME = 16;
	constexpr int MAX_PASSWORD = 32;
	constexpr float INITPOSITION_X[ 3 ] = { 1726.f, 1366.f , 2086.f };
	constexpr float INITPOSITION_Y[ 3 ] = { 1486.f, 2206.f, 2206.f };
	constexpr float INITDIRECTION_X[ 3 ] = { 0.f, -0.5f, -0.5f };
	constexpr float INITDIRECTION_Y[ 3 ] = { 0.f,0.866025f,-0.866025f };
}

namespace InitWorld
{
	// ��� ����
	constexpr int TILE_COUNTX = 7;
	constexpr int TILE_COUNTY = 7;

	// ������
	constexpr float TILECOLLIDER_SIZE = 159.5f;
	constexpr float TILEWITHGAP_SIZE = 360.f; /*��� �� ƴ�� ������ �Ÿ�*/

	//0�� �ε��� ��� ��ġ
	constexpr float FIRST_TILEPOSITION_X = 646.f;
	constexpr float FIRST_TILEPOSITION_Y = 766.f;

	//ù ���� ��� ��ĥ �ε���
	constexpr int FIRSTTILE_COLOR[ 3 ] = { 17,30,32 };

	constexpr int INGAMEPLAYER_NUM = 3;
	constexpr float PLAYERCOLLIDER = 320.f;

	constexpr unsigned char ITEMSPAWNTIME = 20;
}

namespace ItemTypes
{
	//������ ����
	const unsigned char REVERSE_MOVE = 0; //����Ű �ݴ�� ����
}

// PACKET TYPE
// CLIENT
namespace ClientToServer
{
	constexpr unsigned char LOGIN_REQUEST = 0;
	constexpr unsigned char MOVE = 1;
}
// SERVER
namespace ServerToClient
{
	constexpr unsigned char FIRSTINFO = 0;
	constexpr unsigned char LOGON_OK = 1;
	constexpr unsigned char LOGON_FAILED = 2;
	constexpr unsigned char INITPLAYERS = 3;
	constexpr unsigned char MOVE = 4;
	constexpr unsigned char TIME = 5;
	constexpr unsigned char COLLISION_BLOCK = 6;
	constexpr unsigned char COLLISION_PLAYER = 7;
	constexpr unsigned char PLAYERSCORE = 8;
	constexpr unsigned char ITEMSPAWN = 9;

}

#endif // !INITDEFINE_H
