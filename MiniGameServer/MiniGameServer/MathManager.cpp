#include "pch.h"
#include "MathManager.h"
#include <math.h>

float MathManager::Distance2D( const float& x1, const float& y1, const float& x2, const float& y2 )
{
	float distance = sqrt(( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ));
	return distance;
}
