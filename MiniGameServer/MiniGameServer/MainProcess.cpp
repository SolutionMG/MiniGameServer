#include"pch.h"
#include "NetworkManager.h"

int main( )
{
	auto& network = NetworkManager::GetInstance( );
	if ( !network.Initialize( ) )
		return -1;
	
	if ( !network.Listen( ) )
		return -1;

	if ( !network.RunServer( ) )
		return -1;

	return 0;
}