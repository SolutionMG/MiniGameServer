#include "pch.h"
#include "Log.h"


Log::SourceLocation::SourceLocation( const int fileLine, const char* fileName, const char* functionName )
	: fileLine( fileLine ), fileName( fileName ), functionName( functionName )
{
}

Log::SourceLocation::~SourceLocation( ) = default;

void Log::DisplayError(const SourceLocation& sl, const char* msg )
{
	static std::mutex coutLock { };

	void* messageBuffer;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   NULL, WSAGetLastError( ), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				   ( LPTSTR ) &messageBuffer, 0, NULL );

	if ( msg != nullptr )
		std::cout << msg << std::endl;
	std::cout << "[ file: " << sl.fileName << " ] [ func: " << sl.functionName <<" ] [ line: "<<std::to_string(sl.fileLine) <<" ]" << std::endl;
	std::wcout << L"¿¡·¯ " << messageBuffer << std::endl;

}

