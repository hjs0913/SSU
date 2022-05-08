#include "stdafx.h"
#include "Network.h"
#include "Player.h"

#include "GameFramework.h"

int my_id = 0;
int m_prev_size = 0;
vector<string> g_msg;
JOB my_job = J_DILLER;
ELEMENT my_element = E_NONE;

wstring my_name = L"";
wstring my_job_str = L"";
wstring my_element_str = L"";
wstring Info_str = L"";
wstring Combat_str = L"";
bool Combat_On = false;
atomic_bool InDungeon = false;

// locale variable
XMFLOAT3 my_position(-1.0f, 5.0f, -1.0f);
XMFLOAT3 my_camera(0.0f, 0.0f, 0.0f);
WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
int retval = 0;

SOCKET g_s_socket;

WSABUF mybuf_recv;
WSABUF mybuf;

int combat_id = -1;
int party_id[GAIA_ROOM];
wstring party_name[GAIA_ROOM];
CPattern m_gaiaPattern;
int indun_death_count = 4;

array<CPlayer*, MAX_USER+MAX_NPC> mPlayer;
array<Party*, (MAX_USER / GAIA_ROOM)> m_party;
vector<int> party_id_index_vector;
Party* m_party_info;
bool party_info_on = false;
int  robby_cnt = 0;
bool party_enter = false;
int party_enter_room_id = -1;
bool alramUI_ON = false;
bool PartyInviteUI_ON = false;
bool InvitationCardUI_On = false;
chrono::system_clock::time_point InvitationCardTimer = chrono::system_clock::now();
int InvitationRoomId;
int InvitationUser;

struct EXP_OVER {
	WSAOVERLAPPED m_wsa_over;
	WSABUF m_wsa_buf;
	unsigned char m_net_buf[BUFSIZE];
	COMP_OP m_comp_op;
};

EXP_OVER _recv_over;

HANDLE g_h_iocp;	// ���߿� iocp�ٲ� �� ���


bool g_client_shutdown = false;


void err_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	wcout << lpMsgBuf << endl;

	//while (true);
	LocalFree(lpMsgBuf);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void send_login_packet(char* id, char* name)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.id, id);
	strcpy_s(packet.name, name);
	do_send(sizeof(packet), &packet);
}

void send_attack_packet(int skill)
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	//packet.skill = (char)skill;
	do_send(sizeof(packet), &packet);
}

void send_move_packet(XMFLOAT3 position)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	//packet.direction = (char)direction;
	packet.x = position.x;
	packet.y = position.y;
	packet.z = position.z;
	do_send(sizeof(packet), &packet);
}

void send_look_packet(XMFLOAT3 look, XMFLOAT3 right)
{
	cs_packet_look packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOOK;
	packet.x = look.x;
	packet.y = look.y;
	packet.z = look.z;
	packet.right_x = right.x;
	packet.right_y = right.y;
	packet.right_z = right.z;
	do_send(sizeof(packet), &packet);
}

void send_skill_packet(int sk_t, int sk_n)
{
	cs_packet_skill packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_SKILL;

	packet.skill_type = sk_t;
	packet.skill_num = sk_n;
	do_send(sizeof(packet), &packet);
}

void send_picking_skill_packet(int sk_t, int sk_n, int target)
{
	cs_packet_picking_skill packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PICKING_SKILL;
	packet.target = target;
	packet.skill_type = sk_t;
	packet.skill_num = sk_n;
	do_send(sizeof(packet), &packet);

}
void send_change_job_packet(JOB my_job)
{
	cs_packet_change_job packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHANGE_JOB;
	packet.job = my_job;
	do_send(sizeof(packet), &packet);

}
void send_change_element_packet(ELEMENT my_element)
{
	cs_packet_change_element packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHANGE_ELEMENT;
	packet.element = my_element;
	do_send(sizeof(packet), &packet);

}
void send_chat_packet(const char* send_str)
{
	cs_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHAT;
	strcpy_s(packet.message, send_str);
	do_send(sizeof(packet), &packet);
}

void send_party_room_packet()
{
	cs_packet_party_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM;
	do_send(sizeof(packet), &packet);
}

void send_raid_rander_ok_packet()
{
	cs_packet_raid_rander_ok packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_RAID_RANDER_OK;
	do_send(sizeof(packet), &packet);
}

void send_party_room_make()
{
	cs_packet_party_room_make packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_MAKE;
	do_send(sizeof(packet), &packet);
}

void send_party_room_info_request(int r_id)
{
	cs_packet_party_room_info_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_INFO_REQUEST;
	packet.room_id = m_party[r_id]->get_party_id();
	do_send(sizeof(packet), &packet);
}

void send_party_room_enter_request()
{
	cs_packet_party_room_enter_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_ENTER_REQUEST;
	packet.room_id = m_party_info->get_party_id();
	do_send(sizeof(packet), &packet);
}

void send_party_room_quit_request()
{
	cs_packet_party_room_quit_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_QUIT_REQUEST;
	packet.room_id = party_enter_room_id;
	do_send(sizeof(packet), &packet);
}

void send_party_invite(char* user)
{
	cs_packet_party_invite packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_INVITE;
	packet.room_id = party_enter_room_id;
	strcpy_s(packet.user_name, user);
	do_send(sizeof(packet), &packet);
}
void send_party_add_partner()
{
	cs_packet_party_add_partner packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ADD_PARTNER;
	packet.room_id = party_enter_room_id;
	do_send(sizeof(packet), &packet);
}

void send_party_invitation_reply(int accept)
{
	cs_packet_party_invitation_reply packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_INVITATION_REPLY;
	packet.room_id = InvitationRoomId;
	packet.invite_user_id = InvitationUser;
	packet.accept = accept;
	do_send(sizeof(packet), &packet);
}

void do_send(int num_bytes, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER;
	ex_over->m_comp_op = OP_SEND;
	ZeroMemory(&ex_over->m_wsa_over, sizeof(ex_over->m_wsa_over));
	ex_over->m_wsa_buf.buf = reinterpret_cast<char*>(ex_over->m_net_buf);
	ex_over->m_wsa_buf.len = num_bytes;
	memcpy(ex_over->m_net_buf, mess, num_bytes);

	int ret = WSASend(g_s_socket, &ex_over->m_wsa_buf, 1, 0, 0, &ex_over->m_wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num) {
			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, error_num,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, 0);
			wcout << lpMsgBuf << endl;
			LocalFree(lpMsgBuf);
		}
	}
}

void do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over.m_wsa_over, sizeof(_recv_over.m_wsa_over));
	_recv_over.m_wsa_buf.buf = reinterpret_cast<char*>(_recv_over.m_net_buf + m_prev_size);
	_recv_over.m_wsa_buf.len = sizeof(_recv_over.m_net_buf) - m_prev_size;
	int ret = WSARecv(g_s_socket, &_recv_over.m_wsa_buf, 1, 0, &recv_flag, &_recv_over.m_wsa_over, NULL);
	
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num) {
			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, error_num,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, 0);
			wcout << lpMsgBuf << endl;
			//while (true);
			LocalFree(lpMsgBuf);
		}
	}
}

void process_packet(unsigned char* p) 
{
	int type = *(p + 1);
	switch (type) {
	case SC_PACKET_LOGIN_OK: {
		// �÷��̾��� ��� ������ ��������
		cout << "�α��� ����" << endl;
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(p);
		my_id = packet->id;
		my_position.x = packet->x;
		my_position.y = packet->y;
		my_position.z = packet->z;
		mPlayer[my_id]->m_lv = packet->level;
		strcpy_s(mPlayer[my_id]->m_name, MAX_NAME_SIZE, packet->name);
		mPlayer[my_id]->m_hp = packet->hp;
		cout << "�α��� - " << mPlayer[my_id]->m_hp << endl;
		mPlayer[my_id]->m_max_hp = packet->maxhp;
		mPlayer[my_id]->m_mp = packet->mp;
		mPlayer[my_id]->m_max_mp = packet->maxmp;
		mPlayer[my_id]->m_exp = packet->exp;
		mPlayer[my_id]->m_tribe = static_cast<TRIBE>(packet->tribe);
		my_element = packet->element;
		my_job = packet->job;
		

		wchar_t* temp;;
		int len = 1 + strlen(mPlayer[my_id]->m_name);
		temp = new TCHAR[len];
		mbstowcs(temp, mPlayer[my_id]->m_name, len);
		my_name.append(temp);
		delete temp;

		switch (my_element) {
		case E_NONE: my_element_str = L"���Ӽ�"; break;
		case E_WATER: my_element_str = L"��"; break;
		case E_FULLMETAL: my_element_str = L"��ö"; break;
		case E_WIND: my_element_str = L"�ٶ�"; break;
		case E_FIRE: my_element_str = L"��"; break;
		case E_TREE: my_element_str = L"����"; break;
		case E_EARTH: my_element_str = L"��"; break;
		case E_ICE: my_element_str = L"����"; break;
		}
		switch (my_job) {
		case J_DILLER: my_job_str = L"����"; break;
		case J_MAGICIAN: my_job_str = L"������"; break;
		case J_SUPPORTER: my_job_str = L"������"; break;
		case J_TANKER: my_job_str = L"��Ŀ"; break;
		}

		Info_str.append(L"Lv : ");
		Info_str.append(to_wstring(packet->level));
		Info_str.append(L"  �̸� : ");
		Info_str.append(my_name);
		Info_str.append(L"\n���� : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  �Ӽ� : ");
		Info_str.append(my_element_str);

		break;
	}
	case SC_PACKET_MOVE: {
		sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(p);

		if (packet->id == my_id) {
			my_position.x = packet->x;
			my_position.y = packet->y;
			my_position.z = packet->z;
		}
		else {
			mPlayer[packet->id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			//mPlayer[packet->id]->vCenter = XMFLOAT3(packet->x, packet->y, packet->z);
		}
		break;
	}
	case SC_PACKET_PUT_OBJECT: {
		sc_packet_put_object* packet = reinterpret_cast<sc_packet_put_object*> (p);
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) {
			/*if (static_cast<TRIBE>(packet->object_type) == BOSS) {
				p_id += 100;
			}*/

			mPlayer[p_id]->SetUse(true);
			mPlayer[p_id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			//mPlayer[p_id]->vCenter = XMFLOAT3(packet->x, packet->y, packet->z);
			mPlayer[p_id]->SetLook(XMFLOAT3(packet->look_x, packet->look_y, packet->look_z));
			mPlayer[p_id]->m_lv = packet->level;
			mPlayer[p_id]->m_hp = packet->hp;
			mPlayer[p_id]->m_max_hp = packet->maxhp;
			mPlayer[p_id]->m_mp = packet->mp;
			mPlayer[p_id]->m_max_mp = packet->maxmp;
			mPlayer[p_id]->m_element = packet->element;

			mPlayer[p_id]->m_tribe = static_cast<TRIBE>(packet->object_type);
			strcpy_s(mPlayer[p_id]->m_name, packet->name);
			mPlayer[p_id]->m_spices = packet->object_class;
		}
		break;
	}
	case SC_PACKET_REMOVE_OBJECT: {
		sc_packet_remove_object* packet = reinterpret_cast<sc_packet_remove_object*>(p);
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) mPlayer[p_id]->SetUse(false);
		if (p_id == combat_id) {
			combat_id = -1;
			Combat_On = false;
		}

		break;
	}
	case SC_PACKET_CHAT: {
		sc_packet_chat* packet = reinterpret_cast<sc_packet_chat*>(p);
		string msg = "";
		msg += packet->message;
		
		if (g_msg.size() >= 5) g_msg.erase(g_msg.begin());
	
		g_msg.push_back(msg);

		break;
	}
	case SC_PACKET_LOGIN_FAIL: {
		cout << "�α��� ����(3���� �����ϴ�)" << endl;
		Sleep(3000);
		exit(0);
		break;
	}
	case SC_PACKET_STATUS_CHANGE: {
		sc_packet_status_change* packet = reinterpret_cast<sc_packet_status_change*>(p);
		mPlayer[packet->id]->m_lv = packet->level;
		mPlayer[my_id]->m_hp = packet->hp;
		mPlayer[my_id]->m_mp = packet->mp;
		mPlayer[my_id]->m_max_hp = packet->maxhp;
		mPlayer[my_id]->m_max_mp = packet->maxmp;
		mPlayer[my_id]->m_exp = packet->exp;
		my_element = packet->element;
		my_job = packet->job;

		Info_str = L"";
		switch (my_element) {
		case E_NONE: my_element_str = L"���Ӽ�"; break;
		case E_WATER: my_element_str = L"��"; break;
		case E_FULLMETAL: my_element_str = L"��ö"; break;
		case E_WIND: my_element_str = L"�ٶ�"; break;
		case E_FIRE: my_element_str = L"��"; break;
		case E_TREE: my_element_str = L"����"; break;
		case E_EARTH: my_element_str = L"��"; break;
		case E_ICE: my_element_str = L"����"; break;
		}
		switch (my_job) {
		case J_DILLER: my_job_str = L"����"; break;
		case J_MAGICIAN: my_job_str = L"������"; break;
		case J_SUPPORTER: my_job_str = L"������"; break;
		case J_TANKER: my_job_str = L"��Ŀ"; break;
		}

		Info_str.append(L"Lv : ");
		Info_str.append(to_wstring(packet->level));
		Info_str.append(L"  �̸� : ");
		Info_str.append(my_name);
		Info_str.append(L"\n���� : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  �Ӽ� : ");
		Info_str.append(my_element_str);

		break;
	}
	case SC_PACKET_DEAD: {
		
		sc_packet_dead* packet = reinterpret_cast<sc_packet_dead*> (p);
		mPlayer[my_id]->SetUse(false);
		combat_id = -1;
		Combat_On = false;
		cout << "died" << endl;
		break;
		
	}
	case SC_PACKET_REVIVE: {
		// ���� �̱���
		break;
	}
	case SC_PACKET_LOOK: {
		sc_packet_look* packet = reinterpret_cast<sc_packet_look*>(p);
		XMFLOAT3 xmf3Look(packet->x, packet->y, packet->z);
		mPlayer[packet->id]->SetLook(xmf3Look);
		break;
	}
	case SC_PACKET_CHANGE_HP: {
		sc_packet_change_hp* packet = reinterpret_cast<sc_packet_change_hp*>(p);
		mPlayer[packet->id]->m_hp = packet->hp;
		break;
	}
	case SC_PACKET_COMBAT_ID: {
		sc_packet_combat_id* packet = reinterpret_cast<sc_packet_combat_id*>(p);
		if (combat_id != packet->id) {
			Combat_On = true;
			combat_id = packet->id;

			Combat_str = L"";
			Combat_str.append(L"LV.");
			Combat_str.append(to_wstring(mPlayer[combat_id]->m_lv));
			Combat_str.append(L"  ");
			wchar_t* temp;;
			int len = 1 + strlen(mPlayer[combat_id]->m_name);
			temp = new TCHAR[len];
			mbstowcs(temp, mPlayer[combat_id]->m_name, len);
			Combat_str.append(temp);
			delete temp;

			Combat_str.append(L"\n�Ӽ� : ");
			switch (mPlayer[combat_id]->m_element) {
			case E_NONE: my_element_str = Combat_str.append(L"���Ӽ�"); break;
			case E_WATER: my_element_str = Combat_str.append(L"��"); break;
			case E_FULLMETAL: my_element_str = Combat_str.append(L"��ö"); break;
			case E_WIND: my_element_str = Combat_str.append(L"�ٶ�"); break;
			case E_FIRE: my_element_str = Combat_str.append(L"��"); break;
			case E_TREE: my_element_str = Combat_str.append(L"����"); break;
			case E_EARTH: my_element_str = Combat_str.append(L"��"); break;
			case E_ICE: my_element_str = Combat_str.append(L"����"); break;
			}
		}
		break;
	}
	case SC_PACKET_PLAY_SHOOT: {
		sc_packet_play_shoot* packet = reinterpret_cast<sc_packet_play_shoot*>(p);
		shoot = true;	
		hit_check = false;
		break;
	}
	case SC_PACKET_PLAY_EFFECT: {
		sc_packet_play_effect* packet = reinterpret_cast<sc_packet_play_effect*>(p);
		hit_check = true;
		effect_x = packet->x;
		effect_y = packet->y;
		effect_z = packet->z;
		cout << effect_x << endl;
		cout << effect_y << endl;
		cout << effect_z << endl;
		break;
	}
	case SC_PACKET_START_GAIA: {
		PartyUI_On = false;
		party_info_on = false;
		PartyInviteUI_ON = false;
		InvitationCardUI_On = false;

		sc_packet_start_gaia* packet = reinterpret_cast<sc_packet_start_gaia*>(p);
		cout << "�δ����� �����ؾߵ�" << endl;
		combat_id = 101;
		InDungeon = true;
		for (int i = 0; i < GAIA_ROOM; i++) {
			party_id[i] = packet->party_id[i];

			wchar_t* temp;
			int len = 1 + strlen(mPlayer[party_id[i]]->m_name);
			temp = new TCHAR[len];
			mbstowcs(temp, mPlayer[party_id[i]]->m_name, len);
			party_name[i] = L"";
			party_name[i].append(temp);
		}
		break;
	}
	case SC_PACKET_GAIA_PATTERN_ONE: {
		sc_packet_gaia_pattern_one* packet = reinterpret_cast<sc_packet_gaia_pattern_one*>(p);
		m_gaiaPattern.pattern_on[0] = true;
		m_gaiaPattern.set_pattern_one(packet->point_x, packet->point_z);
		break;
	}
	case SC_PACKET_GAIA_PATTERN_FINISH: {
		sc_packet_gaia_pattern_finish* packet = reinterpret_cast<sc_packet_gaia_pattern_finish*>(p);
		m_gaiaPattern.pattern_on[packet->pattern] = false;
		break;
	}
	case SC_PACKET_GAIA_PATTERN_TWO: {
		sc_packet_gaia_pattern_two* packet = reinterpret_cast<sc_packet_gaia_pattern_two*>(p);
		m_gaiaPattern.pattern_on[1] = true;
		m_gaiaPattern.set_pattern_two(packet->point_x, packet->point_z);
		switch ((int)packet->pattern_number) {
		case 0:
			m_gaiaPattern.pattern_two_look = XMFLOAT3(-1, 0, -1);
			break;
		case 1:
			m_gaiaPattern.pattern_two_look = XMFLOAT3(1, 0, -1);
			break;
		case 2:
			m_gaiaPattern.pattern_two_look = XMFLOAT3(1, 0, 1);
			break;
		case 3:
			m_gaiaPattern.pattern_two_look = XMFLOAT3(-1, 0, 1);
			break;
		}
		break;
	}
	case SC_PACKET_GAIA_PATTERN_FIVE: {
		sc_packet_gaia_pattern_five* packet = reinterpret_cast<sc_packet_gaia_pattern_five*>(p);
		m_gaiaPattern.pattern_on[4] = true;
		m_gaiaPattern.pattern_five = XMFLOAT3(packet->point_x, 5, packet->point_z);
		m_gaiaPattern.pattern_five_look = XMFLOAT3(mPlayer[101]->GetLook());
		break;
	}
	case SC_PACKET_CHANGE_DEATH_COUNT: {
		indun_death_count = reinterpret_cast<sc_packet_change_death_count*>(p)->death_count;
		break;
	}
	case SC_PACKET_GAIA_JOIN_OK: {
		break;
	}
	case SC_PACKET_BUFF_UI: {
		sc_packet_buff_ui* packet = reinterpret_cast<sc_packet_buff_ui*>(p);
		switch (packet->buff_num)
		{
		case 0:
			buff_ui_num[0] = packet->buff_num;
			start_buff_0 = clock();
			break;
		case 1:
			buff_ui_num[1] = packet->buff_num;
			start_buff_1 = clock();
			break;
		case 2:
			buff_ui_num[2] = packet->buff_num;
			start_buff_2 = clock();
			break;
		default:
			break;
		}
	

		break;
	}
	case SC_PACKET_PARTY_ROOM: {
		sc_packet_party_room* packet = reinterpret_cast<sc_packet_party_room*>(p);
		m_party[(int)packet->room_id]->set_room_name(packet->room_name);
		if (m_party[(int)packet->room_id]->dst != DUN_ST_ROBBY) {
			m_party[(int)packet->room_id]->dst = DUN_ST_ROBBY;
			robby_cnt++;
			party_id_index_vector.push_back((int)packet->room_id);
		}
		break;
	}
	case SC_PACKET_PARTY_ROOM_INFO: {
		sc_packet_party_room_info* packet = reinterpret_cast<sc_packet_party_room_info*>(p);
		int r_id = (int)packet->room_id;

		m_party[r_id]->player_cnt = 0;
		for (int i = 0; i < packet->players_num; i++) {
			m_party[r_id]->player_id[i] = (int)packet->players_id_in_server[i];
			m_party[r_id]->player_lv[i] = (int)packet->players_lv[i];
			m_party[r_id]->player_job[i] = static_cast<JOB>(packet->players_job[i]);
			switch (i) {
			case 0: m_party[r_id]->set_player_name(packet->player_name1); break;
			case 1: m_party[r_id]->set_player_name(packet->player_name2); break;
			case 2: m_party[r_id]->set_player_name(packet->player_name3); break;
			case 3: m_party[r_id]->set_player_name(packet->player_name4); break;
			}
			m_party[r_id]->player_cnt++;
		}

		for (int i = packet->players_num; i < GAIA_ROOM; i++) {
			m_party[r_id]->set_player_name("");
			m_party[r_id]->player_cnt++;
		}
		m_party[r_id]->player_cnt = packet->players_num;

		PartyUI_On = true;
		party_info_on = true;
		m_party_info = m_party[r_id];
		break;
	}
	case SC_PACKET_PARTY_ROOM_ENTER_OK: {
		party_enter = true;
		party_enter_room_id = reinterpret_cast<sc_packet_party_room_enter_ok*>(p)->room_id;
		break;
	}
	case SC_PACKET_PARTY_ROOM_ENTER_FAILED: {
		int f_reason = (int)reinterpret_cast<sc_packet_party_room_enter_failed*>(p)->failed_reason;
		string msg = "";
		switch (f_reason)
		{
		case 0:
			msg  = "�ο��� ���� �濡 ������ �� �����ϴ�";
			break;
		case 1:
			msg = "�������� �ʴ� ���̹Ƿ� ������ �� �����ϴ�";
			break;
		case 2:
			msg = "�̹� �ٸ��濡 �������Դϴ�";
			break;
		default:
			break;
		}
		if (g_msg.size() >= 5 && (f_reason>=0 && f_reason<=2)) g_msg.erase(g_msg.begin());
		g_msg.push_back(msg);

		party_enter = false;
		break;
	}
	case SC_PACKET_PARTY_ROOM_QUIT_OK: {
		party_enter = false;
		party_info_on = false;
		break;
	}
	case SC_PACKET_PARTY_INVITATION: {
		InvitationRoomId = (int)reinterpret_cast<sc_packet_party_invitation*>(p)->room_id;
		InvitationUser = reinterpret_cast<sc_packet_party_invitation*>(p)->invite_user_id;

		InvitationCardUI_On = true;
		InvitationCardTimer = chrono::system_clock::now() + 10s;
		break;
	}
	case SC_PACKET_PARTY_INVITATION_FAILED: {
		int f_reason = (int)reinterpret_cast<sc_packet_party_invitation_failed*>(p)->failed_reason;
		string msg = "";
		switch (f_reason)
		{
		case 0:
			msg = "���� �α��εǾ����� �ʰų� ���� ����";
			break;
		case 1:
			msg = "(�÷��̾�)";
			msg += reinterpret_cast<sc_packet_party_invitation_failed*>(p)->invited_user;
			msg += "  �ʴ븦 �ź���";
			break;
		case 2:
			msg = "�̹� �ٸ� ��Ƽ�� ��������";
			break;
		default:
			break;
		}
		if (g_msg.size() >= 5 && (f_reason >= 0 && f_reason <= 2)) g_msg.erase(g_msg.begin());
		g_msg.push_back(msg);
		break;
	}
	case SC_PACKET_PARTY_ROOM_DESTROY: {
		m_party[(int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id]->dst = DUN_ST_FREE;
		if (robby_cnt <= 0) robby_cnt = 0;
		else robby_cnt--;

		if (party_id_index_vector.size() == 0) break;

		if (find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id)
			!= party_id_index_vector.end()) {
			int in = find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id) - party_id_index_vector.begin(); // index Ȯ��
			party_id_index_vector.erase(party_id_index_vector.begin()+in);
		}
		break;
	}
	default:
		cout << "�߸��� ��Ŷ type : " << type << endl;
		cout << "Process packet ����" << endl;
		break;
	}
}

void worker()
{
	for (;;) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			err_display(err_no);
			return;
			// ���缭 ��Ʈ��ũ �����尡 ���߸� ��Ʈ��ũ ������ ����Ǿ��ٰ� 
			// LabProject���� ó��
		}
		switch (exp_over->m_comp_op) {
		case OP_RECV: {
			if (num_byte == 0) {
				/*Disconnect(client_id);
				continue;*/
				return;
			}
			unsigned char* packet_start = exp_over->m_net_buf;
			int remain_data = num_byte + m_prev_size;
			int packet_size = packet_start[0];


			//if (packet_size <= 0) break;

			while (packet_size <= remain_data) {
				process_packet(packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				m_prev_size = remain_data;
				memcpy(&exp_over->m_net_buf, packet_start, remain_data);
			}
			if (remain_data == 0) m_prev_size = 0;
			do_recv();
			break;
		}
		case OP_SEND: {
			if (num_byte != exp_over->m_wsa_buf.len) {
				//Disconnect(client_id);
				return;
			}
			delete exp_over;
			break;
		}
		}
	}
}

int netInit()
{
	const char* SERVERIP;
	char tempIP[16];
	SERVERIP = "127.0.0.1";

	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (g_s_socket == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr);
	int ret = connect(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	int err_num = WSAGetLastError();
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : Connect " << endl;
			err_quit("connect()");
		}
	}
	ZeroMemory(&_recv_over, sizeof(_recv_over));
	_recv_over.m_comp_op = OP_RECV;

    g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 0, 0);

	for (auto& pl : mPlayer) {
		pl = new CPlayer();
	}
	
	for (int i = 0; i < (MAX_USER / GAIA_ROOM); i++) {
		m_party[i] = new Party(i);
	}

	char pl_id[MAX_NAME_SIZE];
	char pl_name[MAX_NAME_SIZE];
	cout << "ID�� �Է��ϼ��� : ";
	cin >> pl_id;
	cout << "�̸��� �Է��ϼ��� : ";
	cin >> pl_name;
	send_login_packet(pl_id, pl_name);

	do_recv();
}

int netclose()
{
	// close socket()
	closesocket(sock);

	// ��������
	WSACleanup();
	return 0;
}

XMFLOAT3 return_myPosition() {
	//cout << "position : " << my_position.x << ", " << my_position.y << ", " << my_position.z << endl;
	return my_position;
}

void get_basic_information(CPlayer* m_otherPlayer, int id)
{
	//m_otherPlayer->m_name = ;
	m_otherPlayer->m_hp = mPlayer[id]->m_hp;
	m_otherPlayer->m_max_hp = mPlayer[id]->m_max_hp;
	m_otherPlayer->m_mp = mPlayer[id]->m_mp;
	m_otherPlayer->m_max_mp = mPlayer[id]->m_max_mp;
	m_otherPlayer->m_lv = mPlayer[id]->m_lv;
	m_otherPlayer->m_tribe = mPlayer[id]->m_tribe;
	m_otherPlayer->m_spices = mPlayer[id]->m_spices;
	m_otherPlayer->m_element = mPlayer[id]->m_element;
}

void get_player_information(CPlayer* m_otherPlayer, int id)
{
	m_otherPlayer->m_mp = mPlayer[id]->m_mp;
	m_otherPlayer->m_max_mp = mPlayer[id]->m_max_mp;
	m_otherPlayer->m_physical_attack = mPlayer[id]->m_physical_attack;
	m_otherPlayer->m_physical_defence = mPlayer[id]->m_physical_defence;
	m_otherPlayer->m_magical_attack = mPlayer[id]->m_magical_attack;
	m_otherPlayer->m_magical_defence = mPlayer[id]->m_magical_defence;
	m_otherPlayer->m_basic_attack_factor = mPlayer[id]->m_basic_attack_factor;
	m_otherPlayer->m_defence_factor = mPlayer[id]->m_defence_factor;
	m_otherPlayer->m_move_speed = mPlayer[id]->m_move_speed;
	m_otherPlayer->m_attack_speed = mPlayer[id]->m_attack_speed;
	m_otherPlayer->m_exp = mPlayer[id]->m_exp;
}

XMFLOAT3 return_myCamera() {
	return my_camera;
}

bool get_use_to_server(int id)
{
	return mPlayer[id]->GetUse();
}

XMFLOAT3 get_position_to_server(int id)
{
	return mPlayer[id]->GetPosition();
}

XMFLOAT3 get_look_to_server(int id)
{
	return mPlayer[id]->GetLookVector();
}

int get_hp_to_server(int id)
{
	return mPlayer[id]->m_hp;
}

int get_max_hp_to_server(int id)
{
	return mPlayer[id]->m_max_hp;
}

float get_combat_id_hp()
{
	return mPlayer[combat_id]->m_hp;
}

float get_combat_id_max_hp()
{
	return mPlayer[combat_id]->m_max_hp;
}


wchar_t* get_user_name_to_server(int id)
{
	WCHAR* temp;
	int len = 1 + strlen(mPlayer[id]->m_name);
	temp = new TCHAR[len];
	mbstowcs(temp, mPlayer[id]->m_name, len);
	return temp;
}