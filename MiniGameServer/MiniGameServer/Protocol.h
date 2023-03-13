#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "InitDefine.h"

// PACKET DECLARE
#pragma pack(push, 1)
struct PacketInfo
{
	/*const*/ unsigned char	size;
	/*const*/ unsigned char type;

public:
	PacketInfo() = default;

	PacketInfo(
		const unsigned char	size,
		const unsigned char type )
		: size( size )
		, type( type ) {}
};
namespace Packet
{
	// Ŭ���̾�Ʈ ù ���� �� ������ ���� �ε��� ����
	// Ŭ���̾�Ʈ������ �ش� ��Ŷ�� �ް� owner�� ������ ������ �÷��̾ Set
	struct FirstPlayer
	{
		//const PacketInfo info;
		PacketInfo info;
		int owner;

		FirstPlayer(const int owner)
			: info( sizeof( FirstPlayer ), ServerToClient::FIRSTINFO )
			, owner( owner ) {}
	};

	// �α��� ��û
	// Ŭ���̾�Ʈ������ �ش� ��Ŷ�� ���� �α��� ��û�� ����
	struct LoginRequest
	{
		PacketInfo info;
		char name[ InitPlayer::MAX_NAME ];
		char password[ InitPlayer::MAX_PASSWORD ];
		int owner;

		LoginRequest( const int owner )
			: info( sizeof( LoginRequest ), ClientToServer::LOGIN_REQUEST )
			, owner( owner ), name(),password() {}
	};
	// �α��� ���
	// �������� �α��� ����� Ŭ���̾�Ʈ���� ���� -> ��Ŷ Ÿ���� Login Failed, LoginOk �� �ϳ�
	// ��Ŷ Ÿ��(info.type)�� LoginOk�� �г��� Ŭ���̾�Ʈ ������ �÷��̾ Set
	// LoginFailed�� �α��� ���� ���� ���.
	struct LoginResult
	{
		PacketInfo info;
		int owner;
		int bestScore;

		// �·��� �߰��� �� ����
		LoginResult( const int owner, const int type/*Login Failed, Login Ok*/)
			: info( sizeof( LoginResult ), type ), owner( owner ), bestScore(-1) {}
	};

	// ȸ������ ��û
	struct SignUpRequest
	{
		PacketInfo info;
		int owner;
		char name[ InitPlayer::MAX_NAME ];
		char password[ InitPlayer::MAX_PASSWORD ];

		SignUpRequest( const int owner ) : info( sizeof( SignUpRequest ), ClientToServer::SIGNUP_REQUEST ), owner( owner ), name(), password() {}
	};

	//ȸ������ ��� ����
	struct SignUpResult
	{
		PacketInfo info;
		int owner;

		SignUpResult( const int owner, const unsigned char type ) : info(sizeof( SignUpResult ), type ), owner( owner ) { }
	};

	// �������� �̴ϰ��� �� ��ȯ ��û �� �ʱ�ȭ ���� ����
	// Ŭ���̾�Ʈ���� �ش� ��Ŷ�� �ް� �÷��̾���� �ʱ� ��ġ, ���� ������ Set
	// owner�� ���� �� �÷��̾���� ������ ��
	struct InitPlayers
	{
		PacketInfo info;
		int owner;							/* �÷��̾� ���� */
		char name[ InitPlayer::MAX_NAME ];	/* �÷��̾� �̸� */
		short color;						/* �÷��̾� ���� ���� 0 - red, 1 - blue, 2 - yellow*/
		float x;							/*	x ��ǥ*/
		float y;							/*	y ��ǥ*/
		float directionX;					/*	X���� ����*/
		float directionY;					/*	Y���� ����*/

		// �ʱ� ��ġ �߰��� ����

		InitPlayers( const int owner )
			: info( sizeof( InitPlayers ), ServerToClient::INITPLAYERS )
			, owner( owner ), color(), x(), y(), directionX(), directionY(), name() {}
	};
	
	// Ŭ���̾�Ʈ�� �̵� ��Ŷ �������� ����
	// �������� ��ġ ���� �� �ٸ� Ŭ���̾�Ʈ �鿡�� ����
	// Ŭ���̾�Ʈ�� �ش� ��Ŷ�� ���� ��� owner�� ���� � �÷��̾��� �������� Ȯ�� ��
	// �ش� �÷��̾��� ��ġ, ����, �ӵ��� �����Ͽ� ����
	struct Move
	{
		PacketInfo info;
		int owner;
		float speed;
		float x;
		float y;
		float directionX;
		float directionY;

		Move( const int owner, const unsigned char type/*ClientToServer::Move, ServerToClient::Move*/)
			: info( sizeof( Move ), type )
			, owner( owner ), speed(), x(), y(), directionX(), directionY()	{}
	};

	// �÷��̾�� ��ϰ��� �浹 �߻� �� Ŭ�󿡰� ��Ŷ ����
	struct CollisionTile
	{
		PacketInfo info;
		int owner;
		int tileIndex;

		CollisionTile(const int owner, const int tileIndex)
			:info(sizeof( CollisionTile ), ServerToClient::COLLISION_BLOCK ),owner(owner), tileIndex( tileIndex ) {}
	};

	// �÷��̾�� �÷��̾��� �浹 ��Ŷ
	// �浹�� �߻��� �÷��̾� �迭�� Ŭ���̾�Ʈ���� ����
	// Ŭ���̾�Ʈ�� owners�� -1�� �ƴ� �÷��̾���� lookVector�� �ݴ� �������� ���� ����ŭ �б�
	struct CollisionPlayer
	{
		PacketInfo info;
		int owners[ 3 ];
		int strongers[ 3 ];	/*��ų ��� ���� ����*/

		CollisionPlayer()
			:info( sizeof( CollisionPlayer ), ServerToClient::COLLISION_PLAYER ), owners{ -1,-1,-1 }, strongers{ -1,-1,-1 }  {}
	};

	// �÷��̾�� ���� �浹
	struct CollisionWall
	{
		PacketInfo info;
		int owner;
		unsigned char wallNum;
		float directionX;
		float directionY;

		CollisionWall(const int owner, const unsigned char wallNum)
			: info(sizeof( CollisionWall ), ServerToClient::COLLISION_WALL ),owner(owner), wallNum(wallNum), directionX(), directionY(){}
	};

	// ������ Ŭ���̾�Ʈ���� ������ �ΰ��� Ÿ�̸�
	// ����: 1��
	struct Timer
	{
		PacketInfo info;
		unsigned char time; /*�� ����*/
		Timer(unsigned char time)
			:info(sizeof(Timer), ServerToClient::TIME), time(time) {}
	};

	// �÷��̾� ���� ���� ��Ŷ
	struct Score
	{
		PacketInfo info;
		int owner;
		unsigned char score;
		Score(const int owner, unsigned char score ) :info(sizeof( Score ), ServerToClient::PLAYERSCORE ), owner(owner), score(score){}
	};

	//�÷��̾� �ε��� 
	//�÷��̾� ���� ����
	struct FinishPlayerInfo
	{
		int owner;
		unsigned char score;
	};

	//�÷��̾� ���� ���� �� ���� �շ� �˸� ��Ŷ
	struct EndGame
	{
		PacketInfo info;
		FinishPlayerInfo playerInfo[ 3 ];
		EndGame() : info( sizeof( EndGame ), ServerToClient::ENDGAME ), playerInfo() {};
	};

	//Ŭ�� ��ų ��� ��û �� ��Ŷ
	struct SkillUseRequest
	{
		PacketInfo info;
		SkillUseRequest( ) : info( sizeof( SkillUseRequest ), ClientToServer::SKILLUSE_REQUEST ) {}
	};

	//��ų ��� ��û ��� ���ۿ� ��Ŷ
	struct SkillUseResult
	{
		PacketInfo info;
		int owner; 
		SkillUseResult(const int owner, const unsigned char type ) : info( sizeof( SkillUseResult ), type ), owner(owner) {}
	};

	// ��ų mp ���ſ� ��Ŷ
	struct PlayerMpUpdate
	{
		PacketInfo info;
		int owner;
		unsigned char mp;
		PlayerMpUpdate( const int owner, unsigned char mp ):info(sizeof(PlayerMpUpdate), ServerToClient::MP_UPDATE ), owner(owner), mp(mp){}

	};

	// ��ų ��� ���� ��Ŷ
	struct SkillEnd
	{
		PacketInfo info;
		int owner;
		SkillEnd(const int owner ): info(sizeof( SkillEnd ), ServerToClient::SKILLEND), owner(owner){}
	};

	// ��Ī ��û ��Ŷ
	struct MatchingRequest
	{
		PacketInfo info;
		MatchingRequest() : info( sizeof( MatchingRequest ), ClientToServer::MATHCING_REQUEST ) {}
	};

	// �� ������
	struct QuitRoom
	{
		PacketInfo info;
		QuitRoom( const int matching ) : info( sizeof( QuitRoom ), ClientToServer::QUIT_ROOM ) {}
	};
}
#pragma pack(pop)

#endif // !PROTOCOL_H
