#include "pch.h"
#include "DataBaseManager.h"
#include "Log.h"
#include <string>
#include <locale>
#include <codecvt>

DataBaseManager::DataBaseManager( )
	:m_driver(nullptr), m_connect(nullptr), m_preparedStatement(nullptr), m_result(nullptr)
{
}

DataBaseManager::~DataBaseManager( )
{
	if ( m_result )
		delete m_result;

	if ( m_preparedStatement )
		delete m_preparedStatement;

	if ( m_connect )
		delete m_connect;
}
std::vector<unsigned char> DataBaseManager::UnicodeToUtf8(const std::string& str )
{
	int len = MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, NULL, 0 );
	wchar_t* buffer = new wchar_t[ len ];
	MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, buffer, len );

	std::wstring wide_str( buffer );
	delete[] buffer;

	len = WideCharToMultiByte( CP_UTF8, 0, wide_str.c_str(), wide_str.length(), NULL, 0, NULL, NULL);
	std::vector<unsigned char> mbstr( len );
	WideCharToMultiByte( CP_UTF8, 0, wide_str.c_str(), wide_str.length(), reinterpret_cast<char*>( &mbstr[0] ), len, NULL, NULL);
	return mbstr;
}

std::string DataBaseManager::EncodingUTF8MB4( const std::string& str )
{
	std::string utf8mb4str;
	for ( const auto& c : str ) 
	{
		if ( c <= 0x7f ) 
		{  // ASCII 문자
			utf8mb4str += c;
		}
		else if ( c <= 0x7ff ) 
		{  // 2바이트 문자
			utf8mb4str += ( char )( ( ( c >> 6 ) & 0x1f ) | 0xc0 );
			utf8mb4str += ( char )( ( c & 0x3f ) | 0x80 );
		}
		else if ( c <= 0xffff ) 
		{  // 3바이트 문자
			utf8mb4str += ( char )( ( ( c >> 12 ) & 0x0f ) | 0xe0 );
			utf8mb4str += ( char )( ( ( c >> 6 ) & 0x3f ) | 0x80 );
			utf8mb4str += ( char )( ( c & 0x3f ) | 0x80 );
		}
		else
		{  // 4바이트 문자
			utf8mb4str += ( char )( ( ( c >> 18 ) & 0x07 ) | 0xf0 );
			utf8mb4str += ( char )( ( ( c >> 12 ) & 0x3f ) | 0x80 );
			utf8mb4str += ( char )( ( ( c >> 6 ) & 0x3f ) | 0x80 );
			utf8mb4str += ( char )( ( c & 0x3f ) | 0x80 );
		}
	}
	return utf8mb4str;
}

void DataBaseManager::AnsiToUTF8( const std::string& target, std::string& result )
{
	int len = MultiByteToWideChar( CP_UTF8, 0, target.c_str(), -1, NULL, 0 );
	wchar_t* buffer = new wchar_t[ len ];
	MultiByteToWideChar( CP_UTF8, 0, target.c_str(), -1, buffer, len );
	std::wstring wide_str( buffer );
	delete[] buffer;

	len = WideCharToMultiByte( CP_UTF8, 0, wide_str.c_str(), wide_str.length(), NULL, 0, NULL, NULL );
	result.resize( len, 0 );
	WideCharToMultiByte( CP_UTF8, 0, wide_str.c_str(), wide_str.length(), &result[ 0 ], len, NULL, NULL );
}

#if NDEBUG
bool DataBaseManager::DBConnect( )
{
	setlocale( LC_ALL, "" );
	m_driver = get_driver_instance();
	if ( !m_driver )
	{
		PRINT_LOG( "Database & Server Connect Failed : m_driver == nullptr" );
		return false;
	}

	try{
		sql::ConnectOptionsMap connection_options{};
		connection_options[ "hostName" ] = "tcp://127.0.0.1:3306";
		connection_options[ "userName" ] = "root";
		connection_options[ "password" ] = "487591";
		connection_options[ "schema" ] = "MiniGame";
		connection_options[ "characterSetResults" ] = "utf8mb4";
		connection_options[ "clientCharacterSet" ] = "utf8mb4";
		connection_options[ "OPT_CHARSET_NAME" ] = "utf8mb4";
		connection_options[ "OPT_SET_CHARSET_NAME" ] = "utf8mb4";

		m_connect = m_driver->connect( connection_options );
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
	}

	if ( !m_connect )
	{
		PRINT_LOG( "Database & Server Connect Failed : m_connect == nullptr" );
		return false;
	}

	std::cout << "Database Connect Success..." << std::endl;
	return true;
}
bool DataBaseManager::SignUp( const std::string& name, const std::string& password )
{
	//비밀번호가 공백이거 이름이 공백
	if ( name.empty() || password.empty() )
	{
		//실패 사유 패킷 전송
		return false;
	}
	// 최대 길이 초과
	if ( name.length() > 16 || password.length() > 32 )
	{
		//실패 사유 패킷 전송
		return false;
	}

	//encoding
	//std::vector<unsigned char> returnString = UnicodeToUtf8( name );
	//std::string encodeName( returnString.begin(), returnString.end() );
	//std::cout << encodeName << std::endl;

	//returnString.clear();
	//returnString = UnicodeToUtf8( password );
	//std::string encodePassword( returnString.begin(), returnString.end() );
	//std::cout << encodePassword << std::endl;

	//std::string encodeName = EncodingUTF8MB4(name);
	//std::cout << encodeName << std::endl;
	//std::string encodePassword = EncodingUTF8MB4( password );
	//std::cout << encodePassword << std::endl;

	std::u8string encodeName{ name.begin(), name.end() };
	std::u8string encodePassword{ password.begin(), password.end() };

	std::string reEncodingName{ encodeName.begin(), encodeName.end() };
	std::string reEncodingPassword{ encodePassword.begin(), encodePassword.end() };

	try{
		m_preparedStatement = m_connect->prepareStatement( "SELECT _name FROM t_Player WHERE _name = ?" );
		m_preparedStatement->setString( 1, reEncodingName );
		m_result = m_preparedStatement->executeQuery();
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")"<< std::endl;
	}
	bool returnValue = m_result->next();
	if ( returnValue )
	{
		// 아이디 이미 존재
		//실패 사유 패킷 전송
	}

	delete m_result;
	delete m_preparedStatement;

	// 아이디 생성
	try{
		m_preparedStatement = m_connect->prepareStatement( "INSERT INTO t_Player (_name, _password)  VALUES (?, ?)" );
		m_preparedStatement->setString( 1, name );
		m_preparedStatement->setString( 2, password );
		m_preparedStatement->execute();
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
	}

	delete m_preparedStatement;

	return !returnValue;
}

bool DataBaseManager::LogOn( const std::string& name, const std::string& password, int& bestScore OUT)
{
	//비밀번호가 공백이거 이름이 공백
	if ( name.empty() || password.empty() )
	{
		//실패 사유 패킷 전송
		return false;
	}
	// 최대 길이 초과
	if ( name.length() > 16 || password.length() > 32 )
	{
		//실패 사유 패킷 전송
		return false;
	}

	//encoding
	//std::vector<unsigned char> returnString = UnicodeToUtf8( name );
	//std::string encodeName( returnString.begin(), returnString.end() );
	//std::cout << encodeName << std::endl;

	//returnString.clear();
	//returnString = UnicodeToUtf8( password );
	//std::string encodePassword( returnString.begin(), returnString.end() );
	//std::cout << encodePassword << std::endl;

	//std::string encodeName = EncodingUTF8MB4( name );
	//std::string encodePassword = EncodingUTF8MB4( password );
	// 
	//std::string encodeName;
	//std::string encodePassword;
	//AnsiToUTF8( name, encodeName );
	//AnsiToUTF8( password, encodePassword );

	std::u8string encodeName{ name.begin(), name.end() };
	std::u8string encodePassword{ password.begin(), password.end() };

	std::string reEncodingName{ encodeName.begin(), encodeName.end() };
	std::string reEncodingPassword{ encodePassword.begin(), encodePassword.end() };
	try{
		m_preparedStatement = m_connect->prepareStatement( "SELECT _name, _bestScore FROM t_player WHERE _name = ? AND _password = ?" );
		m_preparedStatement->setString( 1, name );
		m_preparedStatement->setString( 2, password );
		m_result = m_preparedStatement->executeQuery();
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
	}

	bool returnValue = m_result->next();

	if ( !returnValue )
	{
		//이미 존재하는 유저 
		//실패 사유 패킷 전송
	}

	delete m_preparedStatement;
	delete m_result;

	try{
		std::string playerName = m_result->getString( "_name" );
		std::cout << "아이디: " << playerName << std::endl;
		bestScore = m_result->getInt( "_bestScore" );
		std::cout << "최고 점수: " << bestScore << std::endl;
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
	}

	delete m_preparedStatement;
	delete m_result;

	return returnValue;
}

#endif