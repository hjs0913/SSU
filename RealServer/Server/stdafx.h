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
#include <concurrent_priority_queue.h>      //lock�� ���� �ʰ� ������ ť�� ��� ����, atomic�� ->peak�� ����, pop�� ���� trypop�� ����


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