#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include <sql.h>
#include <sqlext.h>

#include <mysql.h>

#include "Singleton.hpp"
class DataBaseManager
	: public Base::TSingleton< DataBaseManager >
{
private:
	SQLHENV m_hEnv;
	SQLHDBC m_hDbc;
	SQLHSTMT m_hStmt;

	SQLWCHAR* m_odbcName;
	SQLWCHAR* m_odbcId;
	SQLWCHAR* m_odbcPw;

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
