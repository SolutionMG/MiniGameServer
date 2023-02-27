#include "pch.h"
#include "DataBaseManager.h"
#include "Log.h"


DataBaseManager::DataBaseManager( )
	: m_odbcName((SQLWCHAR*)L"MiniGame"), m_odbcId((SQLWCHAR*)L"root" ), m_odbcPw( ( SQLWCHAR* ) L"jin980827" )
{
	if ( !DBConnect( ) )
	{
		PRINT_LOG( "DB Connect Failed" );
	}
}

DataBaseManager::~DataBaseManager( )
{
	SQLFreeHandle( SQL_HANDLE_STMT, m_hStmt );
	SQLDisconnect( m_hDbc );
	SQLFreeHandle( SQL_HANDLE_DBC, m_hDbc );
	SQLFreeHandle( SQL_HANDLE_ENV, m_hEnv );
}

bool DataBaseManager::DBConnect( )
{
	if ( SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv ) != SQL_SUCCESS )
	{
		PRINT_LOG( "SQLAllocHandle Failed" );
		return false;
	}

	if ( SQLSetEnvAttr( m_hEnv, SQL_ATTR_ODBC_VERSION, ( SQLPOINTER ) SQL_OV_ODBC3, SQL_IS_INTEGER ) != SQL_SUCCESS )
	{
		PRINT_LOG( "SQLSetEnvAttr Failed" );
		return false;
	}

	// DB 연결
	if ( SQLAllocHandle( SQL_HANDLE_DBC, m_hEnv, &m_hDbc ) != SQL_SUCCESS )
	{
		PRINT_LOG( "SQLAllocHandle Failed" );
		return false;
	}

	if ( SQLConnect( m_hDbc, m_odbcName, SQL_NTS, m_odbcId, SQL_NTS, m_odbcPw, SQL_NTS ) != SQL_SUCCESS )
	{
		PRINT_LOG( "SQLConnect Failed" );
		return false;
	}

	if ( SQLAllocHandle( SQL_HANDLE_STMT, m_hDbc, &m_hStmt ) != SQL_SUCCESS )
	{
		PRINT_LOG( "SQLAllocHandle Failed" );
		return false;
	}

	std::cout << "Database Connect Success..." << std::endl;
	return true;
}

bool DataBaseManager::SignUp( const std::string& name, const std::string& password )
{
	return false;
}

bool DataBaseManager::LogOn( const std::string& name, const std::string& password, int& bestScore )
{
	// 저장 프로시저 호출

	SQLWCHAR* proc_name = ( SQLWCHAR* ) L"{CALL sp_VerificateId(?, ?)}";
	SQLRETURN returnValue = SQLPrepare( m_hStmt, proc_name, SQL_NTS );

	if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
	{
		SQLBindParameter( m_hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 16, 0, ( SQLPOINTER ) name.c_str( ), 16, NULL );
		SQLBindParameter( m_hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0, ( SQLPOINTER ) password.c_str( ), 32, NULL );

		returnValue = SQLExecute( m_hStmt );
		if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
		{
			SQLINTEGER name_length = 0;
			SQLINTEGER score = 0;

			SQLBindCol( m_hStmt, 1, SQL_C_LONG, ( SQLPOINTER ) bestScore, sizeof( bestScore ), NULL );
			returnValue = SQLFetch( m_hStmt );
			if ( returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO )
				return true;
		}
	}
	return false;
}
