#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <array>
#include <queue>

#include "../../RealServer/Server/protocol.h"
#include "stdafx.h"

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

using namespace std;


extern int my_id;
extern int m_prev_recv;
extern vector<string> g_msg;
extern JOB my_job;
extern ELEMENT my_element;

extern wstring my_name;
extern wstring my_job_str;
extern wstring my_element_str;
extern wstring Info_str;
extern wstring Combat_str;
extern bool Combat_On;
extern bool InDungeon;

void err_quit(const char* msg);

void err_display(const char* msg);

void send_attack_packet(int skill);

void send_move_packet(XMFLOAT3 position);

void send_look_packet(XMFLOAT3 look, XMFLOAT3 right);

void send_skill_packet(int sk_t, int sk_n);

void send_chat_packet(const char* send_str);

void send_change_job_packet(JOB my_job);

void send_change_element_packet(ELEMENT my_element);

void send_gaia_join_packet();

int netInit();

void process_packet(unsigned char* p);

int netclose();

void worker();

void do_send(int num_bytes, void* mess);

void do_recv();

// float my_position2 = -1.0f;
XMFLOAT3 return_myPosition();
XMFLOAT3 return_myCamera();


#include "Camera.h"
#include "Object.h"
void get_basic_information(CPlayer* m_otherPlayer, int id);

int get_hp_to_server(int id);

void get_player_information(CPlayer* m_otherPlayer, int id);

bool get_use_to_server(int id);

XMFLOAT3 get_position_to_server(int id);

XMFLOAT3 get_look_to_server(int id);

float get_combat_id_hp();

float get_combat_id_max_hp();

