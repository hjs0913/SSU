#pragma comment(lib, "WS2_32.LIB")
#pragma comment(lib, "MSWsock.LIB")

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <array>
#include "protocol.h"

using namespace std;

void Disconnect(int c_id);

class EXP_OVER
{
public:
	WSAOVERLAPPED	_wsa_over;
	WSABUF			_wsa_buf;
	unsigned char	_net_buf[BUFSIZE];
	COMP_OP			_comp_op;

public:
	EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	EXP_OVER(COMP_OP comp_op) : _comp_op(comp_op) {}

	EXP_OVER()		// array에 넣기 위해서는 기본생성자가 있어야한다
	{
		_comp_op = OP_RECV;
	}

	~EXP_OVER()
	{

	}
};

class CLIENT
{
public:
	EXP_OVER _recv_over;
	SOCKET	_sock;
	int _prev_size;
	int _id;

	bool _use;

	char name[MAX_ID_LEN];
	int x, y;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int exp;
	short attack_factor;
	float defense_factor;
	TRIBE tribe;
public:
	CLIENT() : tribe(T_HUMAN)
	{
		_use = false;
		_prev_size = 0;

		// 일단 임시로 적어놈
		x = 0;
		y = 0;
		level = 50;
		hp = 54000;
		mp = 27500;
		physical_attack = 1250;
		magical_attack = 500;
		physical_defense = 1100;
		magical_defense  = 925;
		attack_factor = 50;
		defense_factor = 0.0002;
		exp = 0;
		element = E_FULLMETAL;
	}
	~CLIENT() 
	{
		closesocket(_sock);
	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
		_recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
		_recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
		int ret = WSARecv(_sock, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
		if (ret == SOCKET_ERROR) {
			int err_num = WSAGetLastError();
			if (ERROR_IO_PENDING != err_num) {
				Disconnect(_id);
			}
		}
	}

	void do_send(int num_bytes, void* mess)
	{
		EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_bytes, mess);
		WSASend(_sock, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
	}
};

class MONSTER 
{
public:
	bool _live;

	int _id;
	char name[MAX_ID_LEN];
	int x, y;
	int hp;
	int physical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int attack_factor;
	float defense_factor;
	TRIBE tribe;

public:
	MONSTER() : tribe(T_MONSTER)
	{
		_live = true;
		_id = 200;

		strcpy_s(name, sizeof(name),"Monster" );
		// 일단 임시로 적어놈
		x = 6;
		y = 6;
		level = 50;
		hp = 54000;
		physical_attack = 1250;
		physical_defense = 1100;
		magical_defense = 925;
		attack_factor = 50;
		defense_factor = 0.0002;
		// element;
	}

	~MONSTER()
	{

	}

};

array<CLIENT, MAX_USER> clients;

array<MONSTER, 1> monsters;

int get_new_id()
{
	static int g_id = 0;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._use == false) {
			clients[i]._use = true;
			return i;
		}
	}

	return -1;
}

void send_remove_object(int c_id, int victim)
{
	sc_packet_logout packet;
	packet.id = victim;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGOUT;
	clients[c_id].do_send(sizeof(packet), &packet);
}

void Disconnect(int c_id)
{
	clients[c_id]._use = false;
	for (auto& cl : clients) {
		if (false == cl._use) continue;
		send_remove_object(cl._id, c_id);
	}
	closesocket(clients[c_id]._sock);
}

void send_login_ok_packet(int c_id)
{
	sc_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN;
	packet.id = c_id;

	packet.x = clients[c_id].x;
	packet.y = clients[c_id].y;
	packet.hp = clients[c_id].hp;
	packet.mp = clients[c_id].mp;
	packet.physical_attack = clients[c_id].physical_attack;
	packet.magical_attack = clients[c_id].magical_attack;
	packet.physical_defense = clients[c_id].physical_defense;
	packet.magical_defense = clients[c_id].magical_defense;
	packet.element = clients[c_id].element;
	packet.level = clients[c_id].level;
	packet.exp = clients[c_id].exp;
	packet.attack_factor = clients[c_id].attack_factor;
	packet.defense_factor = clients[c_id].defense_factor;
	packet.tribe = clients[c_id].tribe;

	clients[c_id].do_send(sizeof(packet), &packet);
}

void send_move_packet(int c_id, int mover)
{
	sc_packet_move packet;
	packet.id = mover;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_MOVE;
	packet.x = clients[mover].x;
	packet.y = clients[mover].y;

	clients[c_id].do_send(sizeof(packet), &packet);
}

void process_packet(int c_id, unsigned char* p)
{
	CLIENT& cl = clients[c_id];
	int size = p[0];
	int type = p[1];

	switch (type) {
	case CS_PACKET_LOGIN: {
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
		// strcpy_s(cl.name, packet->name);
		send_login_ok_packet(c_id);

		// 기존 클라에게 새로 접속한 클라의 정보를 보내줌
		for (auto& other : clients) {
			if (other._id == c_id) continue;
			if (other._use == false) continue;
			sc_packet_put_object packet;
			
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			strcpy_s(packet.name, cl.name);
			packet.id = c_id;

			packet.x = cl.x;
			packet.y = cl.y;
			packet.hp = cl.hp;
			packet.mp = cl.mp;
			packet.physical_attack = cl.physical_attack;
			packet.magical_attack = cl.magical_attack;
			packet.physical_defense = cl.physical_defense;
			packet.magical_defense = cl.magical_defense;
			packet.element = cl.element;
			packet.level = cl.level;
			packet.exp = cl.exp;
			packet.attack_factor = cl.attack_factor;
			packet.defense_factor = cl.defense_factor;
			packet.tribe = cl.tribe;
	
			other.do_send(sizeof(packet), &packet);
		}

		// 새로 접속한 클라에게 현재 월드 상태를 보내줌
		// 1. 접속해 있는 클라
		for (auto& other : clients) {
			if (other._id == c_id) continue;
			if (other._use == false) continue;
			sc_packet_put_object packet;

			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			strcpy_s(packet.name, other.name);
			packet.id = other._id;

			packet.x = other.x;
			packet.y = other.y;
			packet.hp = other.hp;
			packet.mp = other.mp;
			packet.physical_attack = other.physical_attack;
			packet.magical_attack = other.magical_attack;
			packet.physical_defense = other.physical_defense;
			packet.magical_defense = other.magical_defense;
			packet.element = other.element;
			packet.level = other.level;
			packet.exp = other.exp;
			packet.attack_factor = other.attack_factor;
			packet.defense_factor = other.defense_factor;
			packet.tribe = other.tribe;

			cl.do_send(sizeof(packet), &packet);
		}
		// 2. 몬스터의 정보
		for (auto& mon : monsters) {
			if (mon._live == false) continue;
			sc_packet_put_object packet;

			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			strcpy_s(packet.name, mon.name);
			packet.id = mon._id;

			packet.x = mon.x;
			packet.y = mon.y;
			packet.hp = mon.hp;
			packet.mp = 0;
			packet.physical_attack = mon.physical_attack;
			packet.magical_attack = 0;
			packet.physical_defense = mon.physical_defense;
			packet.magical_defense = mon.magical_defense;
			packet.element = mon.element;
			packet.level = mon.level;
			packet.exp = 0;
			packet.attack_factor = mon.attack_factor;
			packet.defense_factor = mon.defense_factor;
			packet.tribe = mon.tribe;

			cl.do_send(sizeof(packet), &packet);
		}

	}break;
	case CS_PACKET_MOVE: {
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
		int x = cl.x;
		int y = cl.y;
		switch (packet->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < WORLD_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < WORLD_WIDTH - 1) x++; break;
		default:
			cout << "Invalid move in client " << c_id << endl;
			exit(-1);
		}
		cl.x = x;
		cl.y = y;
		// 위치가 바뀌었다고 클라에게 알려줌

		for (auto& cl : clients) {
			if (cl._use == true)
				send_move_packet(cl._id, c_id);
		}
		break;



		// 이동이 타당한지 판단한다

		// 이동이 타당하고 이동을 시켰다면 이동한 정보를 모든 클라에게 보내주자

	}break;
	case CS_PACKET_ATTACK: {
		
		// 현재 전투에 들어갈만한 몬스터가 있는지 확인

		// 만약 있다면 몬스터의 정보를 보내주자

		// 다른 클라들에게는 전투에 돌입했다는 정보를 보내주자

	}break;
	}
}

int main()
{
	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// listen 소켓 생성
	SOCKET server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind()
	bind(server_sock, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(server_sock, SOMAXCONN);

	// iocp 핸들 객체 생성
	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	// 핸들에 소켓 연결
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_sock), h_iocp, 0, 0);

	// 클라 소켓 생성 -> AcceptEx
	SOCKET client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	EXP_OVER accept_ex;

	ZeroMemory(&accept_ex, sizeof(accept_ex));
	accept_ex._comp_op = OP_ACCEPT;
	
	AcceptEx(server_sock, client_sock, accept_buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

	while (1) {
		DWORD num_byte;		// 전송받거나 전송될 데이터의 양
		LONG64 iocp_key;	// 미리 정해 놓은 ID
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		// 예외처리 (보통은 연결이 끊어진 것이고 연결이 끊어진것을 처리 해주어야 한다)
		if (ret == FALSE){
			Disconnect(client_id);
			if (exp_over->_comp_op == OP_SEND)
				delete exp_over;			// 잘못 받은것을 삭제해 주자
			continue;
		}

		switch (exp_over->_comp_op) {
		case OP_RECV: {
			CLIENT& cl = clients[client_id];
			int remain_data = num_byte + cl._prev_size;
			unsigned char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];
			
			while (packet_size <= remain_data) {
				// 패킷 처리
				process_packet(client_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			// 나머지가 있으면 처리
			if (0 < remain_data) {
				cl._prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}

			cl.do_recv();

			} break;

		case OP_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {	// 중간에 짤린 경우임
				// DISCONNECT();
			}
			delete exp_over;
			} break;

		case OP_ACCEPT: {
			int new_id = get_new_id();
			CLIENT& cl = clients[new_id];

			// 클라의 정보 초기화
			cl.x = 0;
			cl.y = 0;
			cl.level = 50;
			cl.hp = 54000;
			cl.mp = 27500;
			cl.physical_attack = 1250;
			cl.magical_attack = 500;
			cl.physical_defense = 1100;
			cl.magical_defense = 925;

			// 통신을 위한 변수들 초기화
			cl._id = new_id;
			cl._prev_size = 0;
			cl._recv_over._comp_op = OP_RECV;
			cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
			cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
			ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
			cl._sock = client_sock;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(cl._sock), h_iocp, new_id, 0);
			cl.do_recv();

			// Accept가 완료가 되었으니 다시 accept를 해주어야 한다
			ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
			client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(server_sock, client_sock, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
			} break;
		}

	}

	closesocket(server_sock);
	WSACleanup();
	return 0;
}
