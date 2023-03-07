#include "pch.h"
#include "MathManager.h"
#include <math.h>
#include <random>

float MathManager::Distance2D( const float& x1, const float& y1, const float& x2, const float& y2 )
{
	return static_cast< float >( sqrt( pow( ( x1 - x2 ), 2 ) + pow( ( y1 - y2 ), 2 ) ) );
}

bool MathManager::CollisionPointAndRectangle( const float& pointX, const float& pointY, const float& rectangleX, const float& rectangleY, const float& rectangleSize )
{
	if ( pointX < rectangleX - rectangleSize && pointX > rectangleX + rectangleSize )
		return false;

	if ( pointY < rectangleY - rectangleSize && pointY > rectangleY + rectangleSize )
		return false;

	return true;
}

bool MathManager::CollisionSphere( const float& pointX, const float& pointY, const float& pointX2, const float& pointY2, const float& distance )
{
	
	return ( Distance2D( pointX, pointY, pointX2, pointY2 ) <= distance );
}

int MathManager::randomInteger( const int& min, const int& max )
{
	//min부터 max까지 범위에서 랜덤한 값 추출
	std::random_device rd;
	std::mt19937 gen( rd() );
	std::uniform_int_distribution<> dis( min, max );
	 
	return dis(gen);
}
