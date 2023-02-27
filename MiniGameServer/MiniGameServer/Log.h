#ifndef LOG_H
#define LOG_H


#define _FUNCTION_NAME_ __FUNCTION__
#define _FILENAME_ (strrchr(__FILE__,'\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define SOURCEINFO {__LINE__, _FILENAME_, _FUNCTION_NAME_ }

namespace Log
{
	struct SourceLocation
	{
		int fileLine;
		const char* fileName;
		const char* functionName;

		SourceLocation( const int fileLine, const char* fileName, const char* functionName );
		~SourceLocation( ) noexcept;
	};

	void DisplayError(const SourceLocation& sl, const char* msg = "" );
}

#define PRINT_LOG(msg)               Log::DisplayError(SOURCEINFO, msg)

#endif // !LOG_H
