#include "pch.h"
#include "DataBaseManager.h"
#include "Log.h"


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
#if NDEBUG
bool DataBaseManager::DBConnect( )
{
	m_driver = get_driver_instance();
	if ( !m_driver )
	{
		PRINT_LOG( "Database & Server Connect Failed : m_driver == nullptr" );
		return false;
	}
	m_connect = m_driver->connect( "tcp://127.0.0.1:3306", "root", "487591" );

	if ( !m_connect )
	{
		PRINT_LOG( "Database & Server Connect Failed : m_connect == nullptr" );
		return false;
	}

	// �����ͺ��̽��� ����
	m_connect->setSchema( "MiniGame" );

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
	//��й�ȣ�� �����̰� �̸��� ����
	if ( name.empty() || password.empty() )
	{
		//���� ���� ��Ŷ ����
		return false;
	}
	// �ִ� ���� �ʰ�
	if ( name.length() > 16 || password.length() > 32 )
	{
		//���� ���� ��Ŷ ����
		return false;
	}

	try{
		m_preparedStatement = m_connect->prepareStatement( "SELECT _name FROM t_Player WHERE _name = ? AND _password = ?" );
		m_preparedStatement->setString( 1, name );
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
		// ���̵� �̹� ����
		//���� ���� ��Ŷ ����
	}

	delete m_preparedStatement;
	delete m_result;

	// ���̵� ����
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

	delete m_preparedStatement;
	delete m_result;

	return !returnValue;
}

bool DataBaseManager::LogOn( const std::string& name, const std::string& password, int& bestScore OUT)
{
	//��й�ȣ�� �����̰� �̸��� ����
	if ( name.empty() || password.empty() )
	{
		//���� ���� ��Ŷ ����
		return false;
	}
	// �ִ� ���� �ʰ�
	if ( name.length() > 16 || password.length() > 32 )
	{
		//���� ���� ��Ŷ ����
		return false;
	}
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
		//�̹� �����ϴ� ���� 
		//���� ���� ��Ŷ ����
	}

	delete m_preparedStatement;
	delete m_result;

	try{
		std::string playerName = m_result->getString( "_name" );
		std::cout << "���̵�: " << playerName << std::endl;
		bestScore = m_result->getInt( "_bestScore" );
		std::cout << "�ְ� ����: " << bestScore << std::endl;
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