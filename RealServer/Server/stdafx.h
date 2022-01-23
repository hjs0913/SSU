#pragma once
#include <iostream>
#include <mutex>
#include <unordered_set>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <chrono>
#include <algorithm>
#include <concurrent_priority_queue.h>      //lock를 쓰지 않고 열심히 큐를 사용 가능, atomic함 ->peak가 없음, pop도 없고 trypop만 있음


extern "C" {
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}
#pragma comment (lib, "lua54.lib")

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

#include "protocol.h"

using namespace std;