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
	float Distance2D( const float& x1, const float& y1, const float& x2, const float& y2 );
};

#endif // !MATHMANAGER_H
