#include "stdafx.h"
#include "CNet.h"
#include "Player.h"
#include <thread>

CRITICAL_SECTION cs;
CPattern	m_gaiaPattern;
bool InDungeon = false;
bool PartyInviteUI_On = false;
bool hit_check = false;
int effect_x = 0;
int effect_y = 0;
int effect_z = 0;


CNet::CNet()
{
	// network연결
	netInit();
}

CNet::~CNet()
{

}

int CNet::netInit()
{
	const char* SERVERIP;
	char tempIP[16];
	SERVERIP = "127.0.0.1";

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("Window Socket Initailze failed(Close Program)");

	// socket()
	sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr);
	int ret = connect(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	int err_num = WSAGetLastError();
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			std::cout << " EROOR : Connect " << std::endl;
			err_quit("connect()");
		}
	}
	ZeroMemory(&_recv_over, sizeof(_recv_over));
	_recv_over.m_comp_op = OP_RECV;

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), g_h_iocp, 0, 0);

	for (int i = 0; i < (MAX_USER / GAIA_ROOM); i++) {
		m_party[i] = new Party(i);
	}

	char pl_id[MAX_NAME_SIZE];
	char pl_name[MAX_NAME_SIZE];
	std::cout << "ID를 입력하세요 : ";
	std::cin >> pl_id;
	std::cout << "이름을 입력하세요 : ";
	std::cin >> pl_name;
	send_login_packet(pl_id, pl_name);

	do_recv();

	return 0;
}

int CNet::netclose()
{
	// close socket()
	closesocket(sock);

	// 윈속종료
	WSACleanup();
	return 0;
}

void CNet::err_quit(const char* msg)
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

void CNet::err_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	std::wcout << lpMsgBuf << std::endl;

	//while (true);
	LocalFree(lpMsgBuf);
}

void CNet::do_send(int num_bytes, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER;
	ex_over->m_comp_op = OP_SEND;
	ZeroMemory(&ex_over->m_wsa_over, sizeof(ex_over->m_wsa_over));
	ex_over->m_wsa_buf.buf = reinterpret_cast<char*>(ex_over->m_net_buf);
	ex_over->m_wsa_buf.len = num_bytes;
	memcpy(ex_over->m_net_buf, mess, num_bytes);

	int ret = WSASend(sock, &ex_over->m_wsa_buf, 1, 0, 0, &ex_over->m_wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num) {
			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, error_num,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, 0);
			std::wcout << lpMsgBuf << std::endl;
			LocalFree(lpMsgBuf);
		}
	}
}

void CNet::do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over.m_wsa_over, sizeof(_recv_over.m_wsa_over));
	_recv_over.m_wsa_buf.buf = reinterpret_cast<char*>(_recv_over.m_net_buf + m_prev_size);
	_recv_over.m_wsa_buf.len = sizeof(_recv_over.m_net_buf) - m_prev_size;
	int ret = WSARecv(sock, &_recv_over.m_wsa_buf, 1, 0, &recv_flag, &_recv_over.m_wsa_over, NULL);

	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num) {
			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, error_num,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, 0);
			std::wcout << lpMsgBuf << std::endl;
			//while (true);
			LocalFree(lpMsgBuf);
		}
	}
}

void CNet::GameobjectSynchronize(CGameObject** obj, int m_nobjects)
{
	std::cout << "sex" << std::endl;
	for (int i = 0; i < MAX_USER + MAX_NPC; i++) {
		m_pPlayer[i] = reinterpret_cast<CPlayer*>(obj[m_nobjects - MAX_NPC - MAX_USER +i]);
	}
}

void CNet::MyplayerSynchronize(CPlayer* pl)
{
	while (1) {
		if (my_id != -1) {
			std::cout << my_id << std::endl;
			m_pPlayer[my_id] = pl;
			break;
		}
	}
}

void CNet::process_packet(unsigned char* p)
{
	int type = *(p + 1);
	switch (type) {
	case SC_PACKET_LOGIN_OK: {
		// 플레이어의 모든 정보를 보내주자
		std::cout << "로그인 성공" << std::endl;
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(p);
		my_id = packet->id;
		m_pPlayer[my_id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
		m_pPlayer[my_id]->m_lv = packet->level;
		strcpy_s(m_pPlayer[my_id]->m_name, MAX_NAME_SIZE, packet->name);
		m_pPlayer[my_id]->m_hp = packet->hp;
		m_pPlayer[my_id]->m_max_hp = packet->maxhp;
		m_pPlayer[my_id]->m_mp = packet->mp;
		m_pPlayer[my_id]->m_max_mp = packet->maxmp;
		m_pPlayer[my_id]->m_exp = packet->exp;
		m_pPlayer[my_id]->m_tribe = static_cast<TRIBE>(packet->tribe);
		my_element = packet->element;
		my_job = packet->job;


		wchar_t* temp;;
		int len = 1 + strlen(m_pPlayer[my_id]->m_name);
		temp = new TCHAR[len];
		mbstowcs(temp, m_pPlayer[my_id]->m_name, len);
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

		Info_str.append(L"Lv : ");
		Info_str.append(std::to_wstring(packet->level));
		Info_str.append(L"  이름 : ");
		Info_str.append(my_name);
		Info_str.append(L"\n직업 : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  속성 : ");
		Info_str.append(my_element_str);

		break;
	}
	case SC_PACKET_MOVE: {
		sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(p);
		m_pPlayer[packet->id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
		break;
	}
	case SC_PACKET_PUT_OBJECT: {
		sc_packet_put_object* packet = reinterpret_cast<sc_packet_put_object*> (p);
		int p_id = packet->id;
		std::cout << "P_id : " << p_id << std::endl;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) {
			/*if (static_cast<TRIBE>(packet->object_type) == BOSS) {
				p_id += 100;
			}*/

			m_pPlayer[p_id]->SetUse(true);
			m_pPlayer[p_id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			//mPlayer[p_id]->vCenter = XMFLOAT3(packet->x, packet->y, packet->z);
			m_pPlayer[p_id]->SetLook(XMFLOAT3(packet->look_x, packet->look_y, packet->look_z));
			m_pPlayer[p_id]->m_lv = packet->level;
			m_pPlayer[p_id]->m_hp = packet->hp;
			m_pPlayer[p_id]->m_max_hp = packet->maxhp;
			m_pPlayer[p_id]->m_mp = packet->mp;
			m_pPlayer[p_id]->m_max_mp = packet->maxmp;
			m_pPlayer[p_id]->m_element = packet->element;

			m_pPlayer[p_id]->m_tribe = static_cast<TRIBE>(packet->object_type);
			strcpy_s(m_pPlayer[p_id]->m_name, packet->name);
			m_pPlayer[p_id]->m_spices = packet->object_class;
		}
		break;
	}
	case SC_PACKET_REMOVE_OBJECT: {
		sc_packet_remove_object* packet = reinterpret_cast<sc_packet_remove_object*>(p);
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) m_pPlayer[p_id]->SetUse(false);
		if (p_id == combat_id) {
			combat_id = -1;
			Combat_On = false;
		}

		break;
	}
	case SC_PACKET_CHAT: {
		sc_packet_chat* packet = reinterpret_cast<sc_packet_chat*>(p);
		std::string msg = "";
		msg += packet->message;

		if (g_msg.size() >= 5) g_msg.erase(g_msg.begin());

		g_msg.push_back(msg);

		break;
	}
	case SC_PACKET_LOGIN_FAIL: {
		std::cout << "로그인 실패(3초후 꺼집니다)" << std::endl;
		Sleep(3000);
		exit(0);
		break;
	}
	case SC_PACKET_STATUS_CHANGE: {
		sc_packet_status_change* packet = reinterpret_cast<sc_packet_status_change*>(p);
		m_pPlayer[packet->id]->m_lv = packet->level;
		m_pPlayer[my_id]->m_hp = packet->hp;
		m_pPlayer[my_id]->m_mp = packet->mp;
		m_pPlayer[my_id]->m_max_hp = packet->maxhp;
		m_pPlayer[my_id]->m_max_mp = packet->maxmp;
		m_pPlayer[my_id]->m_exp = packet->exp;
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

		Info_str.append(L"Lv : ");
		Info_str.append(std::to_wstring(packet->level));
		Info_str.append(L"  이름 : ");
		Info_str.append(my_name);
		Info_str.append(L"\n직업 : ");
		Info_str.append(my_job_str);
		Info_str.append(L"  속성 : ");
		Info_str.append(my_element_str);

		break;
	}
	case SC_PACKET_DEAD: {
		sc_packet_dead* packet = reinterpret_cast<sc_packet_dead*> (p);
		m_pPlayer[my_id]->SetUse(false);
		combat_id = -1;
		Combat_On = false;
		std::cout << "died" << std::endl;
		break;

	}
	case SC_PACKET_REVIVE: {
		// 아직 미구현
		break;
	}
	case SC_PACKET_LOOK: {
		sc_packet_look* packet = reinterpret_cast<sc_packet_look*>(p);
		XMFLOAT3 xmf3Look(packet->x, packet->y, packet->z);
		m_pPlayer[packet->id]->SetLook(xmf3Look);
		break;
	}
	case SC_PACKET_CHANGE_HP: {
		sc_packet_change_hp* packet = reinterpret_cast<sc_packet_change_hp*>(p);
		m_pPlayer[packet->id]->m_hp = packet->hp;
		break;
	}
	case SC_PACKET_COMBAT_ID: {
		EnterCriticalSection(&cs);
		sc_packet_combat_id* packet = reinterpret_cast<sc_packet_combat_id*>(p);
		if (combat_id != packet->id) {
			Combat_On = true;
			combat_id = packet->id;

			Combat_str = L"";
			Combat_str.append(L"LV.");
			Combat_str.append(std::to_wstring(m_pPlayer[combat_id]->m_lv));
			Combat_str.append(L"  ");
			wchar_t* temp;;
			int len = 1 + strlen(m_pPlayer[combat_id]->m_name);
			temp = new TCHAR[len];
			mbstowcs(temp, m_pPlayer[combat_id]->m_name, len);
			Combat_str.append(temp);
			delete temp;

			Combat_str.append(L"\n속성 : ");
			switch (m_pPlayer[combat_id]->m_element) {
			case E_NONE: my_element_str = Combat_str.append(L"무속성"); break;
			case E_WATER: my_element_str = Combat_str.append(L"물"); break;
			case E_FULLMETAL: my_element_str = Combat_str.append(L"강철"); break;
			case E_WIND: my_element_str = Combat_str.append(L"바람"); break;
			case E_FIRE: my_element_str = Combat_str.append(L"불"); break;
			case E_TREE: my_element_str = Combat_str.append(L"나무"); break;
			case E_EARTH: my_element_str = Combat_str.append(L"땅"); break;
			case E_ICE: my_element_str = Combat_str.append(L"얼음"); break;
			}
		}
		LeaveCriticalSection(&cs);
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
		std::cout << effect_x << std::endl;
		std::cout << effect_y << std::endl;
		std::cout << effect_z << std::endl;
		break;
	}
	case SC_PACKET_START_GAIA: {
		EnterCriticalSection(&cs);
		PartyUI_On = false;
		party_info_on = false;
		PartyInviteUI_On = false;
		InvitationCardUI_On = false;

		sc_packet_start_gaia* packet = reinterpret_cast<sc_packet_start_gaia*>(p);
		std::cout << "인던으로 입장해야됨" << std::endl;
		combat_id = 101;
		InDungeon = true;
		for (int i = 0; i < GAIA_ROOM; i++) {
			party_id[i] = packet->party_id[i];
			std::cout << party_id[i] << " : " << m_pPlayer[party_id[i]]->m_name << std::endl;
			wchar_t* temp;
			int len = 1 + strlen(m_pPlayer[party_id[i]]->m_name);
			temp = new TCHAR[len];
			mbstowcs(temp, m_pPlayer[party_id[i]]->m_name, len);
			party_name[i] = L"";
			party_name[i].append(temp);
		}
		LeaveCriticalSection(&cs);
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
		m_gaiaPattern.pattern_five_look = XMFLOAT3(m_pPlayer[101]->GetLook());
		break;
	}
	case SC_PACKET_CHANGE_DEATH_COUNT: {
		indun_death_count = reinterpret_cast<sc_packet_change_death_count*>(p)->death_count;
		party_info_str = L"파티원정보(DC : ";
		party_info_str.append(std::to_wstring(indun_death_count));
		party_info_str.append(L")");
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
		m_party[(int)packet->room_id]->dst = DUN_ST_ROBBY;
		robby_cnt++;
		party_id_index_vector.push_back((int)packet->room_id);
		break;
	}
	case SC_PACKET_PARTY_ROOM_INFO: {
		EnterCriticalSection(&cs);
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
		LeaveCriticalSection(&cs);
		break;
	}
	case SC_PACKET_PARTY_ROOM_ENTER_OK: {
		party_enter = true;
		party_enter_room_id = reinterpret_cast<sc_packet_party_room_enter_ok*>(p)->room_id;
		break;
	}
	case SC_PACKET_PARTY_ROOM_ENTER_FAILED: {
		int f_reason = (int)reinterpret_cast<sc_packet_party_room_enter_failed*>(p)->failed_reason;
		std::string msg = "";
		switch (f_reason)
		{
		case 0:
			msg = "인원이 꽉차 방에 입장할 수 없습니다";
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
		if (g_msg.size() >= 5 && (f_reason >= 0 && f_reason <= 2)) g_msg.erase(g_msg.begin());
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
		EnterCriticalSection(&cs);
		InvitationRoomId = (int)reinterpret_cast<sc_packet_party_invitation*>(p)->room_id;
		InvitationUser = reinterpret_cast<sc_packet_party_invitation*>(p)->invite_user_id;

		InvitationCardUI_On = true;

		InvitationCard_str = L"";
		wchar_t* temp;
		int len = 1 + strlen(m_pPlayer[InvitationUser]->m_name);
		temp = new TCHAR[len];
		mbstowcs(temp, m_pPlayer[InvitationUser]->m_name, len);
		InvitationCard_str.append(temp);
		InvitationCard_str.append(L"가 ");
		InvitationCard_str.append(std::to_wstring(InvitationRoomId));
		InvitationCard_str.append(L"번 방에 초대하였습니다");

		InvitationCardTimer = std::chrono::system_clock::now() + std::chrono::seconds(10);
		delete temp;
		LeaveCriticalSection(&cs);
		break;
	}
	case SC_PACKET_PARTY_INVITATION_FAILED: {
		int f_reason = (int)reinterpret_cast<sc_packet_party_invitation_failed*>(p)->failed_reason;
		std::string msg = "";
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
		if (robby_cnt <= 0) robby_cnt = 0;
		else robby_cnt--;

		if (party_id_index_vector.size() == 0) break;

		if (find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id)
			!= party_id_index_vector.end()) {
			int in = find(party_id_index_vector.begin(), party_id_index_vector.end(), (int)reinterpret_cast<sc_packet_party_room_destroy*>(p)->room_id) - party_id_index_vector.begin(); // index 확인
			party_id_index_vector.erase(party_id_index_vector.begin() + in);
		}
		break;
	}
	case SC_PACKET_NOTICE: {
		EnterCriticalSection(&cs);
		sc_packet_notice* packet = reinterpret_cast<sc_packet_notice*>(p);
		NoticeUI_On = true;
		if ((int)packet->raid_enter == 0) {
			RaidEnterNotice = true;
		}
		NoticeTimer = std::chrono::system_clock::now() + std::chrono::seconds(5);

		wchar_t* temp;
		int len = 1 + strlen(packet->message);
		temp = new TCHAR[len];
		mbstowcs(temp, packet->message, len);
		Notice_str = L"";
		Notice_str.append(temp);
		LeaveCriticalSection(&cs);
		break;
	}
	default:
		std::cout << "잘못된 패킷 type : " << type << std::endl;
		std::cout << "Process packet 오류" << std::endl;
		break;
	}
}

void CNet::worker()
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


//===============================================================================================


void CNet::check_timer()
{
	if (InvitationCardUI_On) {
		if (std::chrono::system_clock::now() > InvitationCardTimer) {
			InvitationCardUI_On = false;
			// 초대 거절 패킷 보내기
			send_party_invitation_reply(0);
		}
	}

	if (NoticeUI_On) {
		if (NoticeTimer < std::chrono::system_clock::now()) {
			NoticeUI_On = false;
			RaidEnterNotice = false;
		}
		else {
			if (RaidEnterNotice) {
				auto t = NoticeTimer - std::chrono::system_clock::now();
				if (t >= std::chrono::seconds(4) && t < std::chrono::seconds(5)) Notice_str = L"4초후에 게임을 시작합니다";
				else if (t >= std::chrono::seconds(3) && t < std::chrono::seconds(4)) Notice_str = L"3초후에 게임을 시작합니다";
				else if (t >= std::chrono::seconds(2) && t < std::chrono::seconds(3)) Notice_str = L"2초후에 게임을 시작합니다";
				else if (t >= std::chrono::seconds(1) && t < std::chrono::seconds(2)) Notice_str = L"1초후에 게임을 시작합니다";
			}
		}
	}
}

void CNet::StartWorkerThread()
{
	std::thread hThread{ &CNet::worker, this };
	hThread.detach();
}

//----------------------------------------------------------------------------------------------


int CNet::get_my_id()
{
	return my_id;
}

JOB CNet::get_my_job()
{
	return my_job;
}

ELEMENT CNet::get_my_element()
{
	return my_element;
}

int CNet::get_my_hp()
{
	return m_pPlayer[my_id]->m_hp;
}

int CNet::get_my_max_hp()
{
	return m_pPlayer[my_id]->m_max_hp;
}

int CNet::get_combat_hp()
{
	return m_pPlayer[combat_id]->m_hp;
}

int CNet::get_combat_max_hp()
{
	return m_pPlayer[combat_id]->m_max_hp;
}

Party* CNet::get_party_info()
{
	return m_party_info;
}

int CNet::get_id_hp(int id)
{
	return m_pPlayer[id]->m_hp;
}

int CNet::get_id_max_hp(int id)
{
	return m_pPlayer[id]->m_max_hp;
}

char* CNet::get_party_name(int id)
{
	return m_party[id]->get_room_name();
}

//===============================================================================================
