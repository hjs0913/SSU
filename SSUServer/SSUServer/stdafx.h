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

struct timer_event {
    int obj_id;
    chrono::system_clock::time_point start_time;
    EVENT_TYPE ev;
    /*     target_id
    ��ų ���� ��Ÿ���� ��� : � ��ų���� �־���
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
