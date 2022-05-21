#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <array>
#include <queue>
#include <string>
#include "Player.h"
#include "../RealServer/Server/protocol.h"
#include "stdafx.h"
#include "Pattern.h"
#include "PartyUI.h"

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

using namespace std;

extern array<CPlayer*, MAX_USER + MAX_NPC> mPlayer;
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

extern atomic_bool InDungeon;
extern int party_id[GAIA_ROOM];
extern wstring party_name[GAIA_ROOM];
extern CPattern m_gaiaPattern;
extern int indun_death_count;

extern array<Party*, (MAX_USER / GAIA_ROOM)> m_party;
extern Party* m_party_info;
extern bool PartyUI_On;
extern bool party_info_on;
extern bool AddAIUI_On;
extern int  robby_cnt;
extern vector<int> party_id_index_vector;

extern bool party_enter;
extern int party_enter_room_id;

// 파티 초대할떄 UI
extern bool PartyInviteUI_ON;	

// 초대장 관련 변수(초대 받을떄 UI)
extern bool InvitationCardUI_On;
extern chrono::system_clock::time_point InvitationCardTimer;
extern chrono::system_clock::time_point NoticeTimer;
extern int InvitationRoomId;
extern int InvitationUser;

extern bool NoticeUI_On;
extern bool RaidEnterNotice;
extern bool DeadNotice;
extern wstring Notice_str;

extern CRITICAL_SECTION IndunCheck_cs;
extern CRITICAL_SECTION UI_cs;
void err_quit(const char* msg);

void err_display(const char* msg);

void send_login_packet(char* id, char* name);

void send_attack_packet(int skill);

void send_move_packet(XMFLOAT3 position);

void send_look_packet(XMFLOAT3 look, XMFLOAT3 right);

void send_skill_packet(int sk_t, int sk_n);

void send_picking_skill_packet(int sk_t, int sk_n, int target);

void send_chat_packet(const char* send_str);

void send_change_job_packet(JOB my_job);

void send_change_element_packet(ELEMENT my_element);

void send_party_room_packet();

void send_raid_rander_ok_packet();

void send_party_room_make();

void send_party_room_info_request(int r_id);

void send_party_room_enter_request();

void send_party_room_quit_request();

void send_party_invite(char* user); 

void send_party_add_partner(JOB j);

void send_party_invitation_reply(int accept);

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

void get_object_information(CGameObject* m_otherPlayer, int id);

void get_basic_information(CPlayer* m_otherPlayer, int id);

int get_hp_to_server(int id);

int get_max_hp_to_server(int id);

void get_player_information(CPlayer* m_otherPlayer, int id);

bool get_use_to_server(int id);

XMFLOAT3 get_position_to_server(int id);

XMFLOAT3 get_look_to_server(int id);

float get_combat_id_hp();

float get_combat_id_max_hp();

wchar_t* get_user_name_to_server(int id);

void set_myPosition(XMFLOAT3 pos);

