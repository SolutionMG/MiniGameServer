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
	sql::Driver* m_driver;		/*�����ͺ��̽� ����̹� ����*/
	sql::Connection* m_connect; /*�����ͺ��̽� ���� ����*/
	sql::PreparedStatement* m_preparedStatement;/*�����ͺ��̽� ��� ��û ����*/
	sql::ResultSet* m_result;	/*�����ͺ��̽� ��� ��ȯ ����*/


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
