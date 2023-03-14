#ifndef MATHMANAGER_H
#define MATHMANAGER_H


#include "Singleton.hpp"

class MathManager final
	: public Base::TSingleton< MathManager>
{
public:
	explicit MathManager() = default;
	virtual ~MathManager() = default;

	//Math Function
	float Distance2D( const float x1, const float y1, const float x2, const float y2 );	
	bool CollisionPointAndRectangle( const float pointX, const float pointY, const float rectangleX, const float rectangleY, const float rectangleSize = InitWorld::TILECOLLIDER_SIZE );
	bool CollisionSphere( const float pointX, const float pointY, const float pointX2, const float pointY2, const float distance = InitWorld::PLAYERCOLLIDER );
	int randomInteger( const int min, const int max );
	unsigned char CheckCollisionWall( const float x, const float y, const float left = InitWorld::MINIMUM_X, const float right = InitWorld::MAXIMUM_X, const float forward = InitWorld::MINIMUM_Y, const float backward = InitWorld::MAXIMUM_Y );
};

#endif // !MATHMANAGER_H
