#ifndef PCH_H
#define PCH_H

// 미리 컴파일된 헤더파일

//Network
#include<WS2tcpip.h>
#include<MSWSock.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "mswsock.lib")

//C++
#include<iostream>
#include<algorithm>
#include<unordered_map>
#include<concurrent_priority_queue.h>
#include<vector>
#include<string>
#include<mutex>

//User
#include "Protocol.h"
#include "Define.h"

//DB
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlcppconn.lib")

//WONSY Util
#include "WonSY/WonSY_Log.h"
#include "WonSY/WonSY_Enum.h"


#endif // !PCH_H

