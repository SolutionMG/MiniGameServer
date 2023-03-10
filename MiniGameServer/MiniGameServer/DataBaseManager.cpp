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
std::string DataBaseManager::UnicodeToUtf8( const wchar_t* wstr )
{
	int len = WideCharToMultiByte( CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL );
	std::string mbstr( len, 0 );
	WideCharToMultiByte( CP_UTF8, 0, wstr, -1, &mbstr[ 0 ], len, NULL, NULL );
	return mbstr;
} 
#if NDEBUG
bool DataBaseManager::DBConnect( )
{
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
	std::wstring widechar;

	widechar.assign( name.begin(), name.end() );
	std::string encode;
	encode = UnicodeToUtf8( widechar.c_str() );
	std::cout << encode << std::endl;

	std::string encode2;
	widechar.assign( password.begin(), password.end() );
	encode2 = UnicodeToUtf8( widechar.c_str() );

	try{
		m_preparedStatement = m_connect->prepareStatement( "SELECT _name FROM t_Player WHERE _name = ? AND _password = ?" );
		m_preparedStatement->setString( 1, name  );
		m_preparedStatement->setString( 2, password );
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

	delete m_preparedStatement;

	// 아이디 생성
	try{
		m_preparedStatement = m_connect->prepareStatement( "INSERT INTO t_Player (_name, _password)  VALUES (?, ?)" );
		m_preparedStatement->setString( 1, name );
		m_preparedStatement->setString( 2, password );
		m_preparedStatement->executeUpdate();
	}
	catch ( sql::SQLException& e )
	{
		std::cout << "err: " << e.what();
		std::cout << "(MySQL error code : " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
	}

	delete m_result;
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

	//std::wstring widechar = { name.begin(), name.end() };
	//std::string encode;
	//encode = UnicodeToUtf8( widechar.c_str() );

	//std::string encode2;
	//widechar = { password.begin(), password.end() };
	//encode2 = UnicodeToUtf8( widechar.c_str() );

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