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

struct timer_event {
    int obj_id;
    chrono::system_clock::time_point start_time;
    EVENT_TYPE ev;
    /*     target_id
    스킬 관련 쿨타임의 경우 : 어떤 스킬인지 넣어줌
    */
    int target_id;

    constexpr bool operator < (const timer_event& _left) const
    {
        return (start_time > _left.start_time);
    }

};

struct Coord
{
    Coord() : x(0), z(0) {}
    Coord(float a, float b) : x(a), z(b) {}
    float x;
    float z;
};

bool check_inside(Coord a, Coord b, Coord c, Coord n);

bool isInsideTriangle(Coord a, Coord b, Coord c, Coord n);

typedef pair<int, int> pos;

#define REAL_DISTANCE 10
