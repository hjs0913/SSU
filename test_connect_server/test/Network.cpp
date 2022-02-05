#include "stdafx.h"
#include "Network.h"
#include "Player.h"
//#include "GameFramework.h"

int my_id = 0;
int m_prev_size = 0;

XMFLOAT3 my_position(-1.0f, 5.0f, -1.0f);
XMFLOAT3 my_camera(0.0f, 0.0f, 0.0f);
WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
int retval = 0;

SOCKET g_s_socket;

WSABUF mybuf_recv;
WSABUF mybuf;

struct EXP_OVER {
	WSAOVERLAPPED m_wsa_over;
	WSABUF m_wsa_buf;
	unsigned char m_net_buf[BUFSIZE];
	COMP_OP m_comp_op;
};

EXP_OVER _recv_over;

HANDLE g_h_iocp;	// ���߿� iocp�ٲ� �� ���


bool g_client_shutdown = false;
array<CPlayer*, MAX_USER+MAX_NPC> mPlayer;

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
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(p);
		my_id = packet->id;
		my_position.x = packet->x;
		my_position.y = packet->y;
		my_position.z = packet->z;
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
			cout << "packet [" << packet->id << "] :" << packet->x << "," << packet->y << ", " << packet->z << endl;
			mPlayer[packet->id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
		}
		break;
	}
	case SC_PACKET_PUT_OBJECT: {
		sc_packet_put_object* packet = reinterpret_cast<sc_packet_put_object*> (p);
		cout << packet->id << endl;
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) {
			mPlayer[p_id]->SetUse(true);
			mPlayer[p_id]->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
			mPlayer[p_id]->m_tribe = static_cast<TRIBE>(packet->object_type);
			strcpy(mPlayer[p_id]->m_name, packet->name);
			mPlayer[p_id]->m_spices = packet->object_class;
		}
		break;
	}
	case SC_PACKET_REMOVE_OBJECT: {
		sc_packet_remove_object* packet = reinterpret_cast<sc_packet_remove_object*>(p);
		int p_id = packet->id;
		if (static_cast<TRIBE>(packet->object_type) != OBSTACLE) mPlayer[p_id]->SetUse(false);
		break;
	}
	case SC_PACKET_CHAT: {
		// ���� �̱���
		break;
	}
	case SC_PACKET_STATUS_CHANGE: {
		// ���� �̱���
		break;
	}
	case SC_PACKET_DEAD: {
		
		sc_packet_dead* packet = reinterpret_cast<sc_packet_dead*> (p);
		mPlayer[my_id]->SetUse(false);
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
	default:
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

	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, "Ȳõ��");
	do_send(sizeof(packet), &packet);

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
