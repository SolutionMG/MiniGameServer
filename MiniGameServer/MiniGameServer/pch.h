#ifndef PCH_H
#define PCH_H

// �̸� �����ϵ� �������

//Network
#include<WS2tcpip.h>
#include<MSWSock.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "mswsock.lib")

//C++
#include<iostream>
#include<algorithm>
#include<unordered_map>
#include<vector>
#include<string>
#include<mutex>

//User
#include "Protocol.h"
#include "Define.h"

//DB
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlcppconn.lib")

#endif // !PCH_H

