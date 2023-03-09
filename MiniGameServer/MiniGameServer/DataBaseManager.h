#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "Singleton.hpp"
class DataBaseManager
	: public Base::TSingleton< DataBaseManager >
{
private:
	sql::Driver* m_driver;		/*데이터베이스 드라이버 변수*/
	sql::Connection* m_connect; /*데이터베이스 연결 변수*/
	sql::PreparedStatement* m_preparedStatement;/*테이터베이스 명령 요청 변수*/
	sql::ResultSet* m_result;	/*데이터베이스 결과 반환 변수*/


public:
	explicit DataBaseManager( );
	virtual ~DataBaseManager( );

private:
	bool DBConnect( );

public:
	bool SignUp( const std::string& name, const std::string& password );
	bool LogOn( const std::string& name, const std::string& password, int& bestScore);
};

#endif // !DATABASEMANAGER_H
