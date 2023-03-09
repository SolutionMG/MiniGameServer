#include "pch.h"
#include "DataBaseManager.h"
#include "Log.h"


DataBaseManager::DataBaseManager( )
	:m_driver(nullptr), m_connect(nullptr), m_preparedStatement(nullptr), m_result(nullptr)
{
	if ( !DBConnect( ) )
	{
		PRINT_LOG( "DB Connect Failed" );
	}
}

DataBaseManager::~DataBaseManager( )
{
	if ( m_result )
		delete m_result;

	if ( m_preparedStatement )
		delete m_preparedStatement;

	if ( m_connect )
		delete m_connect;

	if ( !m_driver )
		m_driver->threadEnd();
}

bool DataBaseManager::DBConnect( )
{
	m_driver = get_driver_instance();
	m_connect = m_driver->connect( "tcp://127.0.0.1:3306", "root", "487591" );

	if ( !m_connect )
	{
		PRINT_LOG( "Database & Server Connect Failed" );
		return false;
	}

	// 데이터베이스와 연결
	m_connect->setSchema( "MiniGame" );

	std::cout << "Database Connect Success..." << std::endl;
	return true;
}

bool DataBaseManager::SignUp( const std::string& name, const std::string& password )
{
	name;
	password;
	return false;
}

bool DataBaseManager::LogOn( const std::string& name, const std::string& password, int& bestScore )
{
	// 저장 프로시저 호출

	/*SQLWCHAR* proc_name = ( SQLWCHAR* ) L"{CALL sp_VerificateId(?, ?)}";
	SQLRETURN returnValue = SQLPrepare( m_hStmt, proc_name, SQL_NTS );

	if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
	{
		SQLBindParameter( m_hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 16, 0, ( SQLPOINTER ) name.c_str( ), 16, NULL );
		SQLBindParameter( m_hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0, ( SQLPOINTER ) password.c_str( ), 32, NULL );

		returnValue = SQLExecute( m_hStmt );
		if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
		{
			int score = 0;
			SQLBindCol( m_hStmt, 1, SQL_C_LONG, reinterpret_cast< SQLPOINTER >( score ), sizeof( bestScore ), NULL );
			bestScore = score;
			returnValue = SQLFetch( m_hStmt );
			if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
				return true;
		}
	}*/
	return false;
}
