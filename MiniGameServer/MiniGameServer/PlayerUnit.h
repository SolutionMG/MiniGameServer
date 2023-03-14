#ifndef PLAYERUNIT_H
#define PLAYERUNIT_H


#include "ClientUnit.h"

class RoomUnit;

enum class EPlayerState : char
{
	NORMAL,STRONGER, STUN, END
};

struct Position
{
	float x;
	float y;
public:
	Position() : x( 0.f ), y( 0.f )	{}
	Position( float x, float y ) : x( x ), y( y ) {}
};

class PlayerUnit final : public ClientUnit
{

private:
	std::string		m_name;
	Position		m_position;
	/*�ΰ��� �÷��̾� ���� ��*/
	short			m_color;			
	/*�ΰ��� �� ���� ���ھ�*/
	unsigned char	m_score;
	/*��ų ������*/
	unsigned char	m_mp;
	/*��ų ���� �ð� ����*/
	unsigned char	m_skillDuration;		
	/*���� ���� �ð� ����*/
	unsigned char	m_stunDuration;

	/*���� �ְ�����*/
	int				m_bestScore;
	EPlayerState	m_playerState;

public:
	explicit PlayerUnit( const SOCKET& socket );
	virtual ~PlayerUnit( ) noexcept;

	void Initialize();

	// Get
	const Position&			GetPosition( )				{ return m_position; }
	const short				GetColor()					{ return m_color; }
	const unsigned char		GetScore()					{ return m_score;}
	const unsigned char		GetMp()						{ return m_mp; }
	const unsigned char		GetSkillDuration()			{ return m_skillDuration; }
	const unsigned char		GetStunDuration()			{ return m_stunDuration; }
	const int				GetBestScore()				{ return m_bestScore; }
	const std::string&		GetName()					{ return m_name; }
	const EPlayerState		GetPlayerState()			{ return m_playerState; }

	// Set
	void SetName(const std::string& name )					{ m_name = name; }
	void SetPosition( const Position& position )			{ m_position.x = position.x; m_position.y = position.y; }
	void SetColor( const short color )						{ m_color = color; }
	void SetScore( const unsigned char score )				{ m_score = score; }
	void SetMp( const unsigned char mp )					{ m_mp = mp; }
	void SetSkillDuration( const unsigned char duration )	{ m_skillDuration = duration; }
	void SetBestScore( const int score )					{ m_bestScore = score;	}
	void SetPlayerState( const EPlayerState	 state )		{ m_playerState = state; }
	void SetStunDuration( const unsigned char duration )	{ m_stunDuration = duration; }
};

#endif // !PLAYERUNIT_H
