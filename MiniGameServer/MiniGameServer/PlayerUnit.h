#ifndef PLAYERUNIT_H
#define PLAYERUNIT_H


#include "ClientUnit.h"

class RoomUnit;

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
	short			m_color;			/*인게임 플레이어 고유 색*/
	unsigned char	m_score;			/*인게임 내 현재 스코어*/
	unsigned char	m_mp;				/*게이지*/
	bool			m_stronger;			/*스킬 사용 중 여부*/
	unsigned char	m_skillDuration;	/*스킬 지속 시간 갱신*/
	int				m_bestScore;		/*나의 최고점수*/
public:
	explicit PlayerUnit( const SOCKET& socket );
	virtual ~PlayerUnit( ) noexcept;

	void Initialize();

	// Get
	const Position& GetPosition( )				{ return m_position; }
	const short& GetColor()						{ return m_color; }
	const unsigned char GetScore()				{ return m_score;}
	const unsigned char GetMp()					{ return m_mp; }
	const bool& GetStronger()					{ return m_stronger; }
	const unsigned char& GetSkillDuration()		{ return m_skillDuration; }
	const int& GetBestScore()					{ return m_bestScore; }
	const std::string& GetName()				{ return m_name; }

	// Set
	void SetName(const std::string& name )					{ m_name = name; }
	void SetPosition( const Position& position )			{ m_position.x = position.x; m_position.y = position.y; }
	void SetColor( const short& color )						{ m_color = color; }
	void SetScore( const unsigned char& score )				{ m_score = score; }
	void SetMp( const unsigned char& mp )					{ m_mp = mp; }
	void SetStronger( const bool& stronger )				{ m_stronger = stronger; }
	void SetSkillDuration( const unsigned char& duration )	{ m_skillDuration = duration; }
	void SetBestScore( const int score )					{ m_bestScore = score;	}
};

#endif // !PLAYERUNIT_H
