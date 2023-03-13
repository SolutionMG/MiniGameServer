#include "pch.h"
#include "DataBaseManager.h"
#include "Log.h"
#include <string>
#include <locale>

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

	if ( m_driver )
		m_driver->threadEnd();
}

std::string DataBaseManager::EncodingString(const std::string& str )
{
	wchar_t warr[ InitServer::MAX_CONVERT ];
	MultiByteToWideChar( CP_ACP, 0, ( LPCSTR ) str.c_str(), -1, warr, InitServer::MAX_CONVERT );
	char carr[ InitServer::MAX_CONVERT ];
	memset( carr, '\0', sizeof( carr ) );
	WideCharToMultiByte( CP_UTF8, 0, warr, -1, carr, InitServer::MAX_CONVERT, NULL, NULL );
	return carr;
}

std::string DataBaseManager::DecodingString( const std::string& str )
{
	wchar_t warr[ InitServer::MAX_CONVERT ];
	memset( warr, '\0', sizeof( warr ) );
	MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, warr, InitServer::MAX_CONVERT );
	char carr[ InitServer::MAX_CONVERT ];
	memset( carr, '\0', sizeof( carr ) );
	WideCharToMultiByte( CP_ACP, 0, warr, -1, carr, InitServer::MAX_CONVERT, NULL, NULL );
	return carr;
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
		connection_options[ "hostName" ]					= "tcp://127.0.0.1:3306";
		connection_options[ "userName" ]					= "root";
		connection_options[ "password" ]					= "487591";		//- 회사
		connection_options[ "schema" ]						= "MiniGame";	//- 회사
		//connection_options[ "password" ]					= "jin980827";  //	- 집
		//connection_options[ "schema" ]					= "Minigame";   //	- 집
		connection_options[ "characterSetResults" ]			= "utf8mb4";
		connection_options[ "clientCharacterSet" ]			= "utf8mb4";
		connection_options[ "OPT_CHARSET_NAME" ]			= "utf8mb4";
		connection_options[ "SET_CHARSET_NAME" ]			= "utf8mb4";
		connection_options[ "charset" ]						= "utf8mb4";

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

	m_connect->setClientOption( "OPT_CHARSET_NAME", "utf8mb4" );
	m_connect->setClientOption( "SET_CHARSET_NAME", "utf8mb4" );
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
	
	std::cout << name << std::endl;
	std::cout << password << std::endl;

	//std::string encodeName = EncodingString(name);
	//std::string encodePassword = EncodingString( password );

	try{
		m_preparedStatement = m_connect->prepareStatement( "SELECT _name FROM t_Player WHERE _name = ?" );
		m_preparedStatement->setString( 1, name );

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

	//std::string encodeName = EncodingString( name );
	//std::string encodePassword = EncodingString( password );

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

		return false;
	}

	if ( m_preparedStatement )
		delete m_preparedStatement;
	try{
		std::string playerName = DecodingString( m_result->getString( "_name" ) );
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

	return returnValue;
}

#endif