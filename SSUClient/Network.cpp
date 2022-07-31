#include "stdafx.h"
#include "Network.h"
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
atomic_bool Login_ok = false;

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

wstring party_name[GAIA_ROOM];
CPattern m_gaiaPattern;
int indun_death_count = 4;

array<CPlayer*, MAX_USER + MAX_NPC + MAX_AI + 1> mPlayer{};
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
bool AddAIUI_On = false;
bool NoticeUI_On = false;
bool RaidEnterNotice = false;
bool DeadNotice = false;
bool Login_OK = false;
bool Login_Build_Once = false;;
bool Open_Build_Once = false;;
bool Join_On = false;
bool Fail_On = false;
extern int Fail_Reason = 0;
bool Damage_On = false;
int Damage = 0;

vector<int> vectorDamageID1;
vector<int> vectorDamageID2;
vector<int> vectorDamageID3;

bool JOIN_ID_On = false;
bool JOIN_PASSWORD_On = false;
bool JOIN_NICKNAME_On = false;
bool JOIN_DILLER_On = false;
bool JOIN_TANKER_On = false;
bool JOIN_MAGICIAN_On = false;
bool JOIN_SUPPORTER_On = false;
wstring Notice_str = L"";
chrono::system_clock::time_point InvitationCardTimer = chrono::system_clock::now();
chrono::system_clock::time_point NoticeTimer = chrono::system_clock::now();
chrono::system_clock::time_point BossSkillUiTimer = chrono::system_clock::now();
int InvitationRoomId;
int InvitationUser;

CRITICAL_SECTION IndunCheck_cs;
CRITICAL_SECTION UI_cs;

char pl_id[MAX_NAME_SIZE];
char pl_password[MAX_NAME_SIZE];
char pl_nickname[MAX_NAME_SIZE];
int pl_job = -1;
int pl_element = 0;
CGameFramework					gGameFramework;
struct EXP_OVER {
	WSAOVERLAPPED m_wsa_over;
	WSABUF m_wsa_buf;
	unsigned char m_net_buf[BUFSIZE];
	COMP_OP m_comp_op;
};

EXP_OVER _recv_over;

HANDLE g_h_iocp;	// 나중에 iocp바꿀 시 사용


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

void send_login_packet(char* id, char* password, JOB job, ELEMENT element, char* nickname)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	packet.job = job;
	packet.element = element;
	strcpy_s(packet.password, password);
	strcpy_s(packet.id, id);
	strcpy_s(packet.name, nickname);
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
void send_party_add_partner(JOB j)
{
	cs_packet_party_add_partner packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ADD_PARTNER;
	packet.room_id = party_enter_room_id;
	packet.job = j;
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
void send_relogin_packet(char*  id, char*  password, char* nick_name, int job, int element)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_RE_LOGIN;
	packet.job = job;
	packet.element = element;
	strcpy_s(packet.nickname, nick_name);
	strcpy_s(packet.password, password);
	strcpy_s(packet.id, id);
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
		//gGameFramework.Login_Check_And_Build();

		Login_OK = true;
	

		// 플레이어의 모든 정보를 보내주자
		//cout << "로그인 성공" << endl;
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(p);
		my_id = packet->id;
		my_position.x = packet->x;
		my_position.y = packet->y;
		my_position.z = packet->z;
		mPlayer[my_id]->m_lv = packet->level;
		strcpy_s(mPlayer[my_id]->m_name, MAX_NAME_SIZE, packet->name);
		mPlayer[my_id]->m_hp = packet->hp;
		mPlayer[my_id]->m_max_hp = packet->maxhp;
		mPlayer[my_id]->m_mp = packet->mp;
		mPlayer[my_id]->m_max_mp = packet->maxmp;
		mPlayer[my_id]->m_exp = packet->exp;
		mPlayer[my_id]->m_tribe = static_cast<TRIBE>(packet->tribe);
		mPlayer[my_id]->m_net_attack = false;
		mPlayer[my_id]->m_net_dead = false;
		my_element = packet->element;
		my_job = packet->job;
		mPlayer[my_id]->m_job = my_job;
	
		wchar_t* temp;;
		int len = 1 + strlen(mPlayer[my_id]->m_name);
		temp = new TCHAR[len];
		mbstowcs(temp, mPlayer[my_id]->m_name, len);
		my_name.append(temp);
		delete temp;

		switch (my_element) {
		case E_NONE: my_element_str = L"무속성"; break;
		case E_WATER: my_element_str = L"물"; break;
		case E_FULLMETAL: my_element_str = L"강철"; break;
		case E_WIND: my_element_str = L"바람"; break;
		case E_FIRE: my_element_str = L"불"; break;
		case E_TREE: my_element_str = L"나무"; break;
		case E_EARTH: my_element_str = L"땅"; break;
		case E_ICE: my_element_str = L"얼음"; break;
		}
		switch (my_job) {
		case J_DILLER: my_job_str = L"전사"; break;
		case J_MAGICIAN: my_job_str = L"마법사"; break;
		case J_SUPPORTER: my_job_str = L"서포터"; break;
		case J_TANKER: my_job_str = L"탱커"; break;
		}

		Info_str.append(L"\tLv : ");
		Info_str.append(to_wstring(packet->level));
		Info_str.append(L"  이름 : ");
		Info_str.append(my_name);
		Info_str.append(L"\n\t직업 : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  속성 : ");
		Info_str.append(my_element_str);
		Login_ok = true;
		break;

	}
	case SC_PACKET_MOVE: {
		sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(p);

		if (packet->id == my_id) {
			if ((int)packet->move_right == 0) {
				my_position.x = packet->x;
				my_position.y = packet->y;
				my_position.z = packet->z;
				XMFLOAT3 xmf3Look(packet->lx, packet->ly, packet->lz);
				mPlayer[packet->id]->SetLook(xmf3Look);
			}
		}
		else {
			mPlayer[packet->id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			XMFLOAT3 xmf3Look(packet->lx, packet->ly, packet->lz);
			mPlayer[packet->id]->SetLook(xmf3Look);
			//mPlayer[packet->id]->vCenter = XMFLOAT3(packet->x, packet->y, packet->z);
		}
		break;
	}
	case SC_PACKET_PUT_OBJECT: {
		sc_packet_put_object* packet = reinterpret_cast<sc_packet_put_object*> (p);
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) {
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
			mPlayer[p_id]->m_net_attack = false;
			mPlayer[p_id]->m_net_dead = false;
			mPlayer[p_id]->m_tribe = static_cast<TRIBE>(packet->object_type);
			if (mPlayer[p_id]->m_tribe == HUMAN || mPlayer[p_id]->m_tribe == PARTNER) {
				mPlayer[p_id]->m_job = static_cast<JOB>(packet->object_class);
			}
			else {
				mPlayer[p_id]->m_job = J_DILLER;
			}
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

		sc_packet_login_fail* packet = reinterpret_cast<sc_packet_login_fail*>(p);
		if (packet->reason == 1) { //이유1: 없는 아이디나 비밀번호가 틀리다고 말해주자 
			Fail_On = true;
			Fail_Reason = packet->reason; //이유에 따라서 다른 텍스트 
		}
		else if(packet->reason == 2)  ///이유2: 회원 중 이미 있는 아이디라 실패 
		{
			Fail_On = true;
			Fail_Reason = packet->reason;
		}
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
		mPlayer[my_id]->m_job = packet->job;
		my_element = packet->element;
		my_job = packet->job;

		Info_str = L"";
		switch (my_element) {
		case E_NONE: my_element_str = L"무속성"; break;
		case E_WATER: my_element_str = L"물"; break;
		case E_FULLMETAL: my_element_str = L"강철"; break;
		case E_WIND: my_element_str = L"바람"; break;
		case E_FIRE: my_element_str = L"불"; break;
		case E_TREE: my_element_str = L"나무"; break;
		case E_EARTH: my_element_str = L"땅"; break;
		case E_ICE: my_element_str = L"얼음"; break;
		}
		switch (my_job) {
		case J_DILLER: my_job_str = L"전사"; break;
		case J_MAGICIAN: my_job_str = L"마법사"; break;
		case J_SUPPORTER: my_job_str = L"서포터"; break;
		case J_TANKER: my_job_str = L"탱커"; break;
		}

		Info_str.append(L"\tLv : ");
		Info_str.append(to_wstring(packet->level));
		Info_str.append(L"  이름 : ");
		Info_str.append(my_name);
		Info_str.append(L"\n\t직업 : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  속성 : ");
		Info_str.append(my_element_str);

		break;
	}
	case SC_PACKET_DEAD: {
		sc_packet_dead* packet = reinterpret_cast<sc_packet_dead*> (p);
		if (packet->id == my_id) {
			mPlayer[my_id]->SetUse(false);
			mPlayer[my_id]->m_hp = 0;
			if (!InDungeon) {
				combat_id = -1;
				Combat_On = false;
			}
			mPlayer[packet->id]->m_net_dead = true;
		}
		else {
			if (combat_id == packet->id) {
				combat_id = -1;
				Combat_On = false;
			}
			mPlayer[packet->id]->m_net_dead = true;
		}
		break;
		
	}
	case SC_PACKET_REVIVE: {
		sc_packet_revive* packet = reinterpret_cast<sc_packet_revive*>(p);
		if (packet->id == my_id) {
			mPlayer[my_id]->SetUse(true);
			mPlayer[my_id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			my_position = mPlayer[my_id]->GetPosition();
			mPlayer[my_id]->m_hp = mPlayer[my_id]->m_max_hp;
			mPlayer[my_id]->m_mp = mPlayer[my_id]->m_max_mp;
			mPlayer[my_id]->m_exp = packet->exp;
			mPlayer[my_id]->m_net_dead = false;
		}
		else {
			mPlayer[packet->id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			mPlayer[packet->id]->m_hp = packet->hp;
			mPlayer[packet->id]->m_mp = mPlayer[my_id]->m_max_mp;
			mPlayer[packet->id]->m_net_dead = false;
		}
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
		
		if ((packet->id >= NPC_ID_START || packet->id == GAIA_ID) && packet->damage ) {		// packet->id가 NPC_ID_START보다 크거나 같을 떄(몬스터 일때), 보스일때 데미지 띄워주기
			Damage = round(packet->damage);
			
			Damage_On = true;

			//mPlayer[packet->id]->m_nDamageTime = 0;

			switch (mPlayer[packet->id]->m_nDamageCnt) {
			case 0:
				vectorDamageID1.emplace_back(packet->id);
				++(mPlayer[packet->id]->m_nDamageCnt);
				mPlayer[packet->id]->m_nDamage1 = Damage;

				break;
			case 1:
				vectorDamageID2.emplace_back(packet->id);
				++(mPlayer[packet->id]->m_nDamageCnt);
				mPlayer[packet->id]->m_nDamage2 = Damage;

				break;
			case 2:
				vectorDamageID3.emplace_back(packet->id);
				mPlayer[packet->id]->m_nDamageCnt = 0;
				mPlayer[packet->id]->m_nDamage3 = Damage;
				break;
			}
		}
		break;
	}
	case SC_PACKET_COMBAT_ID: {
		EnterCriticalSection(&UI_cs);
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

			Combat_str.append(L"\n속성 : ");
			switch (mPlayer[combat_id]->m_element) {
			case E_NONE: Combat_str.append(L"무속성"); break;
			case E_WATER: Combat_str.append(L"물"); break;
			case E_FULLMETAL: Combat_str.append(L"강철"); break;
			case E_WIND: Combat_str.append(L"바람"); break;
			case E_FIRE: Combat_str.append(L"불"); break;
			case E_TREE: Combat_str.append(L"나무"); break;
			case E_EARTH: Combat_str.append(L"땅"); break;
			case E_ICE: Combat_str.append(L"얼음"); break;
			}
		}
		LeaveCriticalSection(&UI_cs);
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
		break;
	}
	case SC_PACKET_START_GAIA: {
		EnterCriticalSection(&IndunCheck_cs);
		PartyUI_On = false;
		party_info_on = false;
		PartyInviteUI_ON = false;
		InvitationCardUI_On = false;
		Combat_On = false;

		sc_packet_start_gaia* packet = reinterpret_cast<sc_packet_start_gaia*>(p);
		combat_id = GAIA_ID;
		InDungeon = true;
		for (int i = 0; i < GAIA_ROOM; i++) {
			m_party_info->player_id[i] = packet->party_id[i];
			if (packet->party_id[i] == my_id) m_party_info->myId_in_partyIndex = i;
			wchar_t* temp;
			int len = 1 + strlen(mPlayer[m_party_info->player_id[i]]->m_name);
			temp = new TCHAR[len];
			mbstowcs(temp, mPlayer[m_party_info->player_id[i]]->m_name, len);
			party_name[i] = L"";
			party_name[i].append(temp);
		}
		LeaveCriticalSection(&IndunCheck_cs);
		break;
	}
	case SC_PACKET_GAIA_PATTERN_ONE: {
		sc_packet_gaia_pattern_one* packet = reinterpret_cast<sc_packet_gaia_pattern_one*>(p);
		if(m_gaiaPattern.pattern_on[0] == false) BossSkillUiTimer = chrono::system_clock::now();
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
		if (m_gaiaPattern.pattern_on[1] == false) BossSkillUiTimer = chrono::system_clock::now();
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
		if (m_gaiaPattern.pattern_on[4] == false) BossSkillUiTimer = chrono::system_clock::now();
		m_gaiaPattern.pattern_on[4] = true;
		m_gaiaPattern.pattern_five = XMFLOAT3(packet->point_x, 20.0f, packet->point_z);
		m_gaiaPattern.pattern_five_look = XMFLOAT3(mPlayer[GAIA_ID]->GetLook());
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
		EnterCriticalSection(&UI_cs);
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
		case 3:
			buff_ui_num[3] = packet->buff_num;
			start_buff_3 = clock();
			break;
		case 4:  //여기 m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_fSpeed = 1.3f
			buff_ui_num[4] = packet->buff_num;
			start_buff_4 = clock();
			break;
		default:
			break;
		}
		LeaveCriticalSection(&UI_cs);
		break;
	}
	case SC_PACKET_PARTY_ROOM: {
		sc_packet_party_room* packet = reinterpret_cast<sc_packet_party_room*>(p);
		m_party[(int)packet->room_id]->set_room_name(packet->room_name);
		m_party[(int)packet->room_id]->dst = DUN_ST_ROBBY;
		if (find(party_id_index_vector.begin(), party_id_index_vector.end(), packet->room_id)
			== party_id_index_vector.end()) {
			robby_cnt++;
			party_id_index_vector.push_back((int)packet->room_id);
		}
		break;
	}
	case SC_PACKET_PARTY_ROOM_INFO: {
		if (InDungeon) break;
		EnterCriticalSection(&UI_cs);
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
		LeaveCriticalSection(&UI_cs);
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
			msg  = "인원이 꽉차 방에 입장할 수 없습니다";
			break;
		case 1:
			msg = "존재하지 않는 방이므로 입장할 수 없습니다";
			break;
		case 2:
			msg = "이미 다른방에 참가중입니다";
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
		EnterCriticalSection(&UI_cs);
		InvitationRoomId = (int)reinterpret_cast<sc_packet_party_invitation*>(p)->room_id;
		InvitationUser = reinterpret_cast<sc_packet_party_invitation*>(p)->invite_user_id;

		InvitationCardUI_On = true;
		InvitationCardTimer = chrono::system_clock::now() + 10s;
		LeaveCriticalSection(&UI_cs);
		break;
	}
	case SC_PACKET_PARTY_INVITATION_FAILED: {
		int f_reason = (int)reinterpret_cast<sc_packet_party_invitation_failed*>(p)->failed_reason;
		string msg = "";
		switch (f_reason)
		{
		case 0:
			msg = "현재 로그인되어있지 않거나 없는 유저";
			break;
		case 1:
			msg = "(플레이어)";
			msg += reinterpret_cast<sc_packet_party_invitation_failed*>(p)->invited_user;
			msg += "  초대를 거부함";
			break;
		case 2:
			msg = "이미 다른 파티에 참가중임";
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

		if (party_id_index_vector.size() == 0) break;

		if (find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id)
			!= party_id_index_vector.end()) {
			int in = find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id) - party_id_index_vector.begin(); // index 확인
			party_id_index_vector.erase(party_id_index_vector.begin()+in);
			robby_cnt--;
			if (robby_cnt <= 0) robby_cnt = 0;
		}
		break;
	}
	case SC_PACKET_NOTICE: {
		EnterCriticalSection(&UI_cs);
		NoticeUI_On = true;

		sc_packet_notice* packet = reinterpret_cast<sc_packet_notice*>(p);
		if ((int)packet->raid_enter == 0) {
			RaidEnterNotice = true;
			NoticeTimer = chrono::system_clock::now() + 5s;
		}
		else if ((int)packet->raid_enter == 1) {
			DeadNotice = true;
			if(InDungeon) NoticeTimer = chrono::system_clock::now() + 5s;
			else NoticeTimer = chrono::system_clock::now() + 10s;
		}
		else NoticeTimer = chrono::system_clock::now() + 5s;
		wchar_t* temp;
		int len = 1 + strlen(packet->message);
		temp = new TCHAR[len];
		mbstowcs(temp, packet->message, len);
		Notice_str = L"";
		Notice_str.append(temp);
		LeaveCriticalSection(&UI_cs);
		break;
	}
	case SC_PACKET_CHANGE_MP: {
		sc_packet_change_mp* packet = reinterpret_cast<sc_packet_change_mp*>(p);
		mPlayer[packet->id]->m_mp = packet->mp;
		break;
	}
	case SC_PACKET_MOVE_OPENWORLD: {
		EnterCriticalSection(&IndunCheck_cs);
		sc_packet_move_openworld* packet = reinterpret_cast<sc_packet_move_openworld*>(p);
		PartyUI_On = false;
		party_info_on = false;
		PartyInviteUI_ON = false;
		InvitationCardUI_On = false;
		InDungeon = false;
		party_enter = false;
		my_position.x = packet->x;
		my_position.y = packet->y;
		my_position.z = packet->z;
		LeaveCriticalSection(&IndunCheck_cs);
		break;
	}
	case SC_PACKET_ANIMATION_ATTACK: {
		sc_packet_animation_attack* packet = reinterpret_cast<sc_packet_animation_attack*>(p);
		mPlayer[packet->id]->m_net_attack = true;
		break;
	}
	case SC_PACKET_ANIMATION_SKILL: {
		sc_packet_animation_skill* packet = reinterpret_cast<sc_packet_animation_skill*>(p);
		mPlayer[packet->id]->m_net_skill_animation[packet->animation_skill] = true;
		break;
	}
	default:
		cout << "잘못된 패킷 type : " << type << endl;
		cout << "Process packet 오류" << endl;
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
			// 멈춰서 네트워크 쓰레드가 멈추면 네트워크 접속이 종료되었다고 
			// LabProject에서 처리
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
	//SERVERIP = "127.0.0.1";
	SERVERIP = "116.47.180.110";
	// 윈속 초기화
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
	
	for (int i = 0; i < 1/*(MAX_USER / GAIA_ROOM)*/; i++) {
		m_party[i] = new Party(i);
	}
	do_recv();
}

int netclose()
{
	// close socket()
	closesocket(sock);

	// 윈속종료
	WSACleanup();
	return 0;
}

XMFLOAT3 return_myPosition() {
	return my_position;
}

void get_raid_initialize_position(CGameObject* m_otherPlayer, int id)
{
	if (id == 3) {
		m_otherPlayer->SetPosition(get_position_to_server(GAIA_ID));
		m_otherPlayer->SetLook(mPlayer[GAIA_ID]->GetLook());
	}
	else {
		int tmp_id = 0;
		if (id >= m_party_info->myId_in_partyIndex) tmp_id = id + 1;
		else tmp_id = id;

		m_otherPlayer->SetPosition(get_position_to_server(m_party_info->player_id[tmp_id]));
		m_otherPlayer->SetLook(mPlayer[m_party_info->player_id[tmp_id]]->GetLook());
	}
}

void get_raid_information(CGameObject* m_otherPlayer, int id)
{
	int tmp_id = 0;
	if (id >= m_party_info->myId_in_partyIndex) tmp_id = id + 1;
	else tmp_id = id;

	// Death
	if (mPlayer[m_party_info->player_id[tmp_id]]->m_net_dead == true) {	// 사망 애니메이션 출력
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_bDie = true;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[6]->m_fPosition = 0.0f;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(6, true);
		}
		return;
	}
	
	// Revive
	if (m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {	// 부활
		m_otherPlayer->m_pSkinnedAnimationController->m_bDie = false;
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	}

	// Look vector
	if (mPlayer[m_party_info->player_id[tmp_id]]->GetLook().x != m_otherPlayer->GetLook().x ||
		mPlayer[m_party_info->player_id[tmp_id]]->GetLook().y != m_otherPlayer->GetLook().y ||
		mPlayer[m_party_info->player_id[tmp_id]]->GetLook().z != m_otherPlayer->GetLook().z
		) {
		m_otherPlayer->SetLook(mPlayer[m_party_info->player_id[tmp_id]]->GetLook());
	}

	// Move vector
	if (mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().x != m_otherPlayer->GetPosition().x
		|| mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().z != m_otherPlayer->GetPosition().z) {
		if (abs(m_otherPlayer->GetPosition().x - mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().x) >= 100 ||
			abs(m_otherPlayer->GetPosition().z - mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().z) >= 100) { // 좌표 이상 -> 강제수정
			m_otherPlayer->SetPosition(get_position_to_server(m_party_info->player_id[tmp_id]));
		}
		else {
			if (sqrt(pow(m_otherPlayer->GetPosition().x - mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().x, 2) +
				pow(m_otherPlayer->GetPosition().z - mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().z, 2)) < 1.0) { //움직일 거리가 얼마 되지 않음
				if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[1].m_bEnable) { // 이동 애니메이션
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, true);
				}
				m_otherPlayer->SetPosition(get_position_to_server(m_party_info->player_id[tmp_id]));
			}
			else {
				if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[1].m_bEnable) { //이동애니메이션
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, true);
				}
				XMFLOAT3 shiftDirection = Vector3::Normalize(XMFLOAT3(
					mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().x - m_otherPlayer->GetPosition().x,
					0,
					mPlayer[m_party_info->player_id[tmp_id]]->GetPosition().z - m_otherPlayer->GetPosition().z));
				m_otherPlayer->Move(shiftDirection, false);
			}
		}
	}
	else { // Idle animation
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_bEnable
			&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable
			&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable
			&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable
			&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable) { //IDLE
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		}
	}

	if (mPlayer[m_party_info->player_id[tmp_id]]->m_net_attack == true) {
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fPosition = 0.f;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
			mPlayer[m_party_info->player_id[tmp_id]]->m_net_attack = false;
		}
	}


	for (int i = 0; i < 3; i++) {
		if (mPlayer[m_party_info->player_id[tmp_id]]->m_net_skill_animation[i] == true) {
			if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3 + i].m_bEnable) {
				m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i+3]->m_fPosition = 0.f;
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(i + 3, i + 3);
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(i + 3, true);
				mPlayer[m_party_info->player_id[tmp_id]]->m_net_skill_animation[i] = false;
			}
		}
	}
}

void get_gaia_information(CGameObject* m_otherPlayer)
{
	if (mPlayer[GAIA_ID]->GetLook().x != m_otherPlayer->GetLook().x ||
		mPlayer[GAIA_ID]->GetLook().y != m_otherPlayer->GetLook().y ||
		mPlayer[GAIA_ID]->GetLook().z != m_otherPlayer->GetLook().z
		) {
		m_otherPlayer->SetLook(mPlayer[GAIA_ID]->GetLook());
	}

	if (mPlayer[GAIA_ID]->GetPosition().x != m_otherPlayer->GetPosition().x
		|| mPlayer[GAIA_ID]->GetPosition().z != m_otherPlayer->GetPosition().z) {
		if (abs(m_otherPlayer->GetPosition().x - mPlayer[GAIA_ID]->GetPosition().x) >= 100 ||
			abs(m_otherPlayer->GetPosition().z - mPlayer[GAIA_ID]->GetPosition().z) >= 100) {
			m_otherPlayer->SetPosition(get_position_to_server(GAIA_ID));
		}
		else {
			if (sqrt(pow(m_otherPlayer->GetPosition().x - mPlayer[GAIA_ID]->GetPosition().x, 2) +
				pow(m_otherPlayer->GetPosition().z - mPlayer[GAIA_ID]->GetPosition().z, 2)) < 1.0) {
				m_otherPlayer->SetPosition(get_position_to_server(GAIA_ID));
			}
			else {
				XMFLOAT3 shiftDirection = Vector3::Normalize(XMFLOAT3(
					mPlayer[GAIA_ID]->GetPosition().x - m_otherPlayer->GetPosition().x,
					0,
					mPlayer[GAIA_ID]->GetPosition().z - m_otherPlayer->GetPosition().z));
				m_otherPlayer->Move(shiftDirection, false);
			}
		}
	}
	else m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
}

// 몬스터
void get_object_information(CGameObject* m_otherPlayer, int id) 
{
	if (mPlayer[id]->GetUse() == false) {
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_otherPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		return;
	}

	// Death
	if (mPlayer[id]->m_net_dead == true) {	
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_bDie = true;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition = 0.0f;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(4, true);
		}
		return;
	}

	// Revive
	if (m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable) {
		m_otherPlayer->m_pSkinnedAnimationController->m_bDie = false;
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	}

	// Look vector
	if (mPlayer[id]->GetLook().x != m_otherPlayer->GetLook().x ||
		mPlayer[id]->GetLook().y != m_otherPlayer->GetLook().y ||
		mPlayer[id]->GetLook().z != m_otherPlayer->GetLook().z
		) {
		m_otherPlayer->SetLook(mPlayer[id]->GetLook());
	}

	// Move vector
	if (mPlayer[id]->GetPosition().x != m_otherPlayer->GetPosition().x || mPlayer[id]->GetPosition().z != m_otherPlayer->GetPosition().z) {
		if (abs(m_otherPlayer->GetPosition().x - mPlayer[id]->GetPosition().x) >= 100 ||
			abs(m_otherPlayer->GetPosition().z - mPlayer[id]->GetPosition().z) >= 100) {	// 좌표 이상 -> 강제 수정
			m_otherPlayer->SetPosition(get_position_to_server(id));
		}
		else {
			if (sqrt(pow(m_otherPlayer->GetPosition().x - mPlayer[id]->GetPosition().x, 2) +
				pow(m_otherPlayer->GetPosition().z - mPlayer[id]->GetPosition().z, 2)) < 1.0) { // 움직이는 거리가 얼마 되지 않음
				if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {	// 이동 애니메이션
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
				}
				m_otherPlayer->SetPosition(get_position_to_server(id));
			}
			else {
				if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {	// 이동 애니메이션
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
				}
				XMFLOAT3 shiftDirection = Vector3::Normalize(XMFLOAT3(
					mPlayer[id]->GetPosition().x - m_otherPlayer->GetPosition().x ,
					0,
					mPlayer[id]->GetPosition().z - m_otherPlayer->GetPosition().z));
				m_otherPlayer->Move(shiftDirection, false);
			}
		}
		//m_otherPlayer->SetPosition(get_position_to_server(id));
	}
	else { // Idle Animation
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_bEnable
			&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable) {	// IDLE
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		}
	}

	// Attack
	if (mPlayer[id]->m_net_attack == true) {	// 공격 애니메이션
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3]->m_fPosition = 0.f;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(3, true);
			mPlayer[id]->m_net_attack = false;

			//-사운드
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(3, true);
			m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKeys(3, 4);
	

#ifdef _WITH_SOUND_RESOURCE
			m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 0.1f, _T("Animal_attack"));
#else
			if (MAX_USER < id &&  id < MAX_USER + 30) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/토끼_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(3)->SoundPlay(false);
			}
			else if (MAX_USER + 30< id && id < MAX_USER + 60) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/거미_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(4)->SoundPlay(false);
			}
			else if (MAX_USER + 60 < id && id < MAX_USER + 90) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/개구리_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(5)->SoundPlay(false);
			}
			else if (MAX_USER + 90 < id && id < MAX_USER + 120) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/원숭이_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(6)->SoundPlay(false);
			}
			else if (MAX_USER + 120 < id && id < MAX_USER + 150) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/늑대_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(7)->SoundPlay(false);
			}
			else if (MAX_USER + 150 < id && id < MAX_USER + 180) {
				//m_otherPlayer->m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/맷돼지_울음소리.wav"));
				SoundManager::GetSoundManager()->GetSound(8)->SoundPlay(false);
			}

		}
#endif
		CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
		m_otherPlayer->m_pSkinnedAnimationController->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);

	}

}

// 나
void get_basic_information(CPlayer* m_otherPlayer, int id)
{
	m_otherPlayer->m_hp = mPlayer[id]->m_hp;
	m_otherPlayer->m_max_hp = mPlayer[id]->m_max_hp;
	m_otherPlayer->m_mp = mPlayer[id]->m_mp;
	m_otherPlayer->m_max_mp = mPlayer[id]->m_max_mp;
	m_otherPlayer->m_lv = mPlayer[id]->m_lv;
	m_otherPlayer->m_tribe = mPlayer[id]->m_tribe;
	m_otherPlayer->m_spices = mPlayer[id]->m_spices;
	m_otherPlayer->m_element = mPlayer[id]->m_element;
	m_otherPlayer->m_exp = mPlayer[id]->m_exp;

	if (mPlayer[id]->m_net_dead == true) {	// 사망 애니메이션 출력
		if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_bDie = true;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[6]->m_fPosition = 0.0f;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(6, true);
		}
		return;
	}

	if (m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
		m_otherPlayer->m_pSkinnedAnimationController->m_bDie = false;
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	}

	if (mPlayer[id]->m_net_attack == true) {
		mPlayer[id]->m_net_attack = false;
		reinterpret_cast<CTerrainPlayer*>(m_otherPlayer)->Attack(true);
	}

	for (int i = 0; i < 3; i++) {
		if (mPlayer[id]->m_net_skill_animation[i] == true) {
			if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3 + i].m_bEnable) {
				m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i + 3]->m_fPosition = 0.f;
				reinterpret_cast<CTerrainPlayer*>(m_otherPlayer)->Skill(i);
				mPlayer[id]->m_net_skill_animation[i] = false;
			}
		}
	}
}

// 다른 플레이어
void get_player_information(CGameObject* m_otherPlayer, int id)
{
	if (m_otherPlayer->m_pSkinnedAnimationController->m_nAnimationTracks) {
		if (mPlayer[id]->GetUse() == false || id == my_id) {
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_otherPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
			return;
		}

		// Death
		if (mPlayer[id]->m_net_dead == true) {	
			if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
				m_otherPlayer->m_pSkinnedAnimationController->m_bDie = true;
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
				m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[6]->m_fPosition = 0.0f;
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(6, true);
			}
			return;
		}

		// Revive
		if (m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
			m_otherPlayer->m_pSkinnedAnimationController->m_bDie = false;
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		}

		// Look vector
		if (mPlayer[id]->GetLook().x != m_otherPlayer->GetLook().x ||
			mPlayer[id]->GetLook().y != m_otherPlayer->GetLook().y ||
			mPlayer[id]->GetLook().z != m_otherPlayer->GetLook().z
			) {
			m_otherPlayer->SetLook(mPlayer[id]->GetLook());
		}

		// Move vector
		if (mPlayer[id]->GetPosition().x != m_otherPlayer->GetPosition().x || mPlayer[id]->GetPosition().z != m_otherPlayer->GetPosition().z) {
			if (abs(m_otherPlayer->GetPosition().x - mPlayer[id]->GetPosition().x) >= 100 ||
				abs(m_otherPlayer->GetPosition().z - mPlayer[id]->GetPosition().z) >= 100) {	// 좌표 이상 -> 강제수정
				m_otherPlayer->SetPosition(get_position_to_server(id));
			}
			else {
				if (sqrt(pow(m_otherPlayer->GetPosition().x - mPlayer[id]->GetPosition().x, 2) +
					pow(m_otherPlayer->GetPosition().z - mPlayer[id]->GetPosition().z, 2)) < 1.0) {	// 움직일 거리가 얼마 되지 않는다
					if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[1].m_bEnable) {	// 이동 애니메이션
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, true);
					}
					m_otherPlayer->SetPosition(get_position_to_server(id));
				}
				else {
					if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[1].m_bEnable) {	// 이동 애니메이션
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
						m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, true);
					}
					XMFLOAT3 shiftDirection = Vector3::Normalize(XMFLOAT3(
						mPlayer[id]->GetPosition().x - m_otherPlayer->GetPosition().x,
						0,
						mPlayer[id]->GetPosition().z - m_otherPlayer->GetPosition().z));
					m_otherPlayer->Move(shiftDirection, false);
				}
			}
			//m_otherPlayer->SetPosition(get_position_to_server(id));
		}
		else {	// Idle Animation
			if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_bEnable
				&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable
				&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable
				&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable
				&& !m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable) {	// IDLE
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			}
		}

		if (mPlayer[id]->m_net_attack == true) {	// 공격 애니메이션
			// 0에서 시작이 되지 않음.
			if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {
				m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fPosition = 0.f;
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
				m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
				mPlayer[id]->m_net_attack = false;

			}
		}

		for (int i = 0; i < 3; i++) {
			if (mPlayer[id]->m_net_skill_animation[i] == true) {
				if (!m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[3 + i].m_bEnable) {
					m_otherPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3+i]->m_fPosition = 0.f;
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAllDisable();
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(i + 3, i + 3);
					m_otherPlayer->m_pSkinnedAnimationController->SetTrackEnable(i + 3, true);
					mPlayer[id]->m_net_skill_animation[i] = false;
				}
			}
		}
	}
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

JOB get_job(int id)
{
	int tmp_id = 0;
	if (id >= m_party_info->myId_in_partyIndex) tmp_id = id + 1;
	else tmp_id = id;

	return mPlayer[m_party_info->player_id[tmp_id]]->m_job;
}

void set_myPosition(XMFLOAT3 pos)
{
	my_position.x = pos.x;
	my_position.y = pos.y;
	my_position.z = pos.z;
}