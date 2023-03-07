#include "pch.h"
#include "RoomUnit.h"

RoomUnit::RoomUnit()
	:m_time( 0 )
{
	m_players.reserve( 3 );
	m_blocks.reserve( InitWorld::TILE_COUNTX * InitWorld::TILE_COUNTY );

	for ( int i = 0; i < InitWorld::TILE_COUNTY; ++i )
	{
		for ( int j = 0; j < InitWorld::TILE_COUNTX; ++j )
		{
			Tile block(	InitWorld::FIRST_TILEPOSITION_X + ( static_cast< float >( j ) * InitWorld::TILEWITHGAP_SIZE ),
								InitWorld::FIRST_TILEPOSITION_Y + ( static_cast< float >( i ) * InitWorld::TILEWITHGAP_SIZE ) );
			block.index = j + (i * InitWorld::TILE_COUNTX);
			m_blocks.emplace_back( block );
		}
	}
}

void RoomUnit::InitializeRoom()
{
	for ( auto& block : m_blocks )
	{
		block.color = 0;
	}

	m_players.clear();
	m_time =  0 ;
}
