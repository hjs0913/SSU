#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <array>
#include <queue>
#include <array>

#include "../../RealServer/Server/protocol.h"
#include "stdafx.h"
#include "Pattern.h"
#include "PartyUI.h"

#include "Player.h"


struct EXP_OVER {
	WSAOVERLAPPED m_wsa_over;
	WSABUF m_wsa_buf;
	unsigned char m_net_buf[BUFSIZE];
	COMP_OP m_comp_op;
};

extern CRITICAL_SECTION cs;
extern CPattern	m_gaiaPattern;
extern 	bool InDungeon;
extern bool PartyInviteUI_On;		// 파티 초대할떄 쓰는 UI
extern bool hit_check;
extern int effect_x;
extern int effect_y;
extern int effect_z;


class CNet
{
private:
// Network(IOCP)
	WSADATA wsa;
	SOCKET sock;
	EXP_OVER _recv_over;
	int m_prev_size;
	HANDLE g_h_iocp;
	int retval = 0;

	WSABUF mybuf_recv;
	WSABUF mybuf;

	int my_id = -1;
	int combat_id = -1;

	CPlayer* m_pPlayer[MAX_USER+MAX_NPC];
// Client
	int temp_nObjects = 0;

// timer
	std::chrono::system_clock::time_point InvitationCardTimer;
	std::chrono::system_clock::time_point NoticeTimer;

// Party_data
	std::array<Party*, (MAX_USER / GAIA_ROOM)> m_party;
	int party_enter_room_id = -1;

// party Invitation
	int InvitationRoomId;
	int InvitationUser;

// InDungeon
	int			indun_death_count = 4;
	Party		*m_party_info;

public:
// my_information
	JOB my_job = J_DILLER;
	ELEMENT my_element = E_NONE;

	std::wstring my_name = L"";
	std::wstring my_job_str = L"";
	std::wstring my_element_str = L"";
	std::wstring Info_str = L"";			// name + job + element string

// client
	bool shoot = false;

// UI Str
	std::vector<std::string> g_msg;	// 채팅 str
	std::wstring Notice_str = L"";		// 알림창 str
	std::wstring Combat_str = L"";
	std::wstring InvitationCard_str = L"";
	std::wstring party_info_str = L"파티원정보(DC : ";

// UI Bool
	bool Combat_On = false;
	bool PartyUI_On = false;			// 왼쪽 파티 목록
	bool party_info_on = false;			// 우측 파티정보
	bool AddAIUI_On = false;			// AI추가할때 뜨는 UI
	bool InvitationCardUI_On = false;	// 파티 초대받을때 쓰는 UI
	bool NoticeUI_On = false;			// 알림창 UI
	bool RaidEnterNotice = false;		// 알림창이 레이드 시작을 알리는 문구인지 아닌지

// Party_data
	bool party_enter = false;
	int  robby_cnt = 0;		// 현재 만들어진 방이 몇개인지 알려준다
	std::wstring party_name[GAIA_ROOM];
	int party_id[GAIA_ROOM];
	std::vector<int> party_id_index_vector;

public:
// network
	CNet();
	~CNet();
	int netInit();
	int netclose();
	void err_quit(const char* msg);
	void err_display(int err_no);

	void do_send(int num_bytes, void* mess);
	void do_recv();

	void process_packet(unsigned char* p);
	void worker();

	void GameobjectSynchronize(CGameObject** obj, int m_nobjects);
	void MyplayerSynchronize(CPlayer* pl);

// process
	void StartWorkerThread(); 
	void check_timer();
// get
	//(my)
	int get_my_id();
	JOB get_my_job();
	ELEMENT get_my_element();

	//(hp)
	int get_my_hp();
	int get_my_max_hp();
	int get_combat_hp();
	int get_combat_max_hp();
	int get_id_hp(int id);
	int get_id_max_hp(int id);

	//(party)
	Party* get_party_info();
	char* get_party_name(int id);

// send function
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
};

