#include "pch.h"
#include "MathManager.h"
#include <math.h>

float MathManager::Distance2D( const float& x1, const float& y1, const float& x2, const float& y2 )
{
	float distance = sqrt(( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ));
	return distance;
}

bool MathManager::CollisionPointAndRectangle( const float& pointX, const float& pointY, const float& rectangleX, const float& rectangleY, const float& rectangleSize )
{
	if ( pointX < rectangleX - rectangleSize )
		return false;

	if ( pointX > rectangleX + rectangleSize )
		return false;

	if ( pointY < rectangleY - rectangleSize )
		return false;

	if ( pointY > rectangleY + rectangleSize )
		return false;

	return true;
}
