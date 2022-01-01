#pragma comment(lib, "WS2_32.LIB")
#pragma comment(lib, "MSWsock.LIB")

#include <iostream>
#include <array>
#include <thread>
#include <mutex>
#include <chrono>
#include <thread>
#include <vector>
#include <concurrent_priority_queue.h>
#include "stdafx.h"

// ��������
SOCKET server_sock;
HANDLE h_iocp;

using namespace std;

void Disconnect(int c_id);

array<CLIENT, MAX_USER> clients;

array<MONSTER, 1> monsters;

enum EVENT_TYPE {
	EVENT_NPC_MOVE, EVENT_NPC_ATTACK, EVENT_AUTO_PLAYER_HP,
	EVENT_PLAYER_REVIVE, EVENT_NPC_REVIVE, EVENT_PLAYER_ATTACK,
	EVENT_SKILL_COOLTIME
};

struct timer_event {
	int obj_id;
	chrono::system_clock::time_point start_time;
	EVENT_TYPE ev;
	/*     target_id
	��ų ���� ��Ÿ���� ��� : � ��ų���� �־���
	NPC�� �������� ��� : ��׷β��� �÷��̾� id�� �־���
	*/
	int target_id;

	constexpr bool operator < (const timer_event& _left) const
	{
		return (start_time > _left.start_time);
	}

};
concurrency::concurrent_priority_queue<timer_event> timer_queue;

void element_buf(int c_id, int m_id)
{
	CLIENT& cl = clients[c_id];
	MONSTER& mon = monsters[m_id];
	switch (cl.element)
	{
	case E_WATER: {
		if (mon.element == E_FULLMETAL || mon.element == E_FIRE || mon.element == E_EARTH) {
			mon.nuff_element.buf_setting(B_MAGATTACK, 10.0f, 10.0f);
			mon.nuff_element._use = true;
		}
		break;
	}
	case E_FULLMETAL: {
		if (mon.element == E_ICE || mon.element == E_TREE || mon.element == E_WIND) {
			cl.buff_element.buf_setting(B_PHYDEFENCE, 10.0f, 10.0f);
			cl.buff_element._use = true;
		}
		break;
	}
	case E_WIND: {
		if (mon.element == E_WATER || mon.element == E_EARTH || mon.element == E_FIRE) {
			cl.buff_element.buf_setting(B_SPEED, 5.0f, 6.0f);
			cl.buff_element._use = true;
		}
		break;
	}
	case E_FIRE: {
		if (mon.element == E_ICE || mon.element == E_TREE || mon.element == E_FULLMETAL) {
			mon.nuff_element.buf_setting(B_BURN, mon.physical_attack * 0.1f, 10.0f);
			mon.nuff_element._use = true;
		}
		break;
	}
	case E_TREE: {
		if (mon.element == E_EARTH || mon.element == E_WATER || mon.element == E_WIND) {
			mon.nuff_element.buf_setting(B_PHYATTACK, 10.0f, 10.0f);
			mon.nuff_element._use = true;
		}
		break;
	}
	case E_EARTH: {
		if (mon.element == E_ICE || mon.element == E_FULLMETAL || mon.element == E_FIRE) {
			cl.buff_element.buf_setting(B_MAGDEFENCE, 10.0f, 10.0f);
			cl.buff_element._use = true;
		}
		break;
	}
	case E_ICE: {
		if (mon.element == E_TREE || mon.element == E_WATER || mon.element == E_WIND) {
			mon.nuff_element.buf_setting(B_SPEED, 10.0f, 10.0f);
			mon.nuff_element._use = true;
		}
		break;
	}

	default:
		break;
	}
}

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

void send_remove_object(int c_id, int victim, TRIBE tribe, bool died)
{
	if((tribe == T_HUMAN && !died) || tribe == T_MONSTER){
		sc_packet_logout packet;
		packet.id = victim;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_LOGOUT;
		packet.tribe = tribe;
		clients[c_id].do_send(sizeof(packet), &packet);
	}
	else {
		sc_packet_died packet;
		packet.id = victim;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_DIED;
		clients[c_id].do_send(sizeof(packet), &packet);
	}
}

void Disconnect(int c_id)
{
	clients[c_id]._use = false;
	for (auto& cl : clients) {
		if (false == cl._use) continue;
		send_remove_object(cl._id, c_id, T_HUMAN, false);
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

void send_combat_packet(int c_id, int m_id, TRIBE subject)
{
	CLIENT& cl = clients[c_id];
	MONSTER& mon = monsters[m_id];
	
	if (subject == T_HUMAN) {	// ��ü�� �޸�
		if (cl.buff_element._use == false)
			element_buf(c_id, m_id);

		// ������ ��� ����
		mon.do_attack = true;
		int damage;
		if (cl.buff_element._type == B_PHYATTACK) damage = (cl.physical_attack*(1+cl.buff_element._effect/100.0f)) * cl.attack_factor;
		else damage = cl.physical_attack * cl.attack_factor;
		float def_temp = mon.defense_factor * mon.physical_defense;
		int real_damage = int(damage * (1.0f - (def_temp) / (1.0f + def_temp)));
		mon.hp -= real_damage;

		// ȭ�鿡 ǥ��
		cout << "�÷��̾� -> ���� ������ : " << real_damage <<  endl;
		cout << c_id << " �÷��̾� Hp : " << cl.hp << endl;
		cout << "���� Hp : " << mon.hp << endl;

		// ������ ���� ������ ��Ŷ�� ��� ������
		sc_packet_attack packet;
		packet.id = c_id;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_ATTACK;
		packet.damage_size = real_damage;
		packet.p_hp = cl.hp;
		packet.m_hp = mon.hp;
		packet.subject = subject;

		cl.do_send(sizeof(packet), &packet);

		if (mon.hp < 0) {
			mon._live = false;
			// ���Ͱ� ��� �������� ������ �Ǿ�� �Ѵ�
			for (auto& cl : clients)
				send_remove_object(cl._id, m_id, T_MONSTER, false);
		}
	}
	else {	// ��ü�� MONSTER

		if (cl.hp > 0) {   //�߰�   ������� ���� ������� 
			// �Ӽ��ο�
			// ������ ��� ����
			int damage = mon.physical_attack * mon.attack_factor;
			float def_temp;
			if(cl.buff_element._type == B_PHYDEFENCE) def_temp = (cl.defense_factor*(1 + cl.buff_element._effect/100.0f)) * cl.physical_defense;
			else def_temp = cl.defense_factor * cl.physical_defense;
			int real_damage = int(damage * (1.0f - (def_temp) / (1.0f + def_temp)));


			cl.hp -= real_damage;

			if (cl.hp < 0)  //�߰� hp�� ������ 0���� ���� 
				cl.hp = 0;


			// ȭ�鿡 ǥ��
			cout << "���� -> �÷��̾� ������ : " << real_damage << endl;
			cout << c_id << "�� �÷��̾� Hp : " << cl.hp << endl;  //�߰� ��� �÷��̾ �������޴��� ���� 
			cout << "���� Hp : " << mon.hp << endl;

			// ������ ���� ������ ��Ŷ�� ��� ������
			sc_packet_attack packet;
			packet.id = c_id;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_ATTACK;
			packet.damage_size = real_damage;
			packet.p_hp = cl.hp;
			packet.m_hp = mon.hp;
			packet.subject = subject;

			cl.do_send(sizeof(packet), &packet);

			if (cl.hp <= 0) {  //�߰�	
				mon.do_attack = false;
				//for (auto& cls : clients)
				//	if(cls._use == true)
				//		send_remove_object(cls._id, c_id, T_HUMAN, true);   //���� Ŭ�� ������ 
				cout << c_id << "�� �÷��̾� ���" << endl;
				cl.hp = 54000;
				cl.x = 0;
				cl.y = 0;
				for (auto& cls : clients) {
					if (cls._use == true)
						send_move_packet(cls._id, c_id);
				}
			}
		}
		
	}

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

		// ���� Ŭ�󿡰� ���� ������ Ŭ���� ������ ������
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

		// ���� ������ Ŭ�󿡰� ���� ���� ���¸� ������
		// 1. ������ �ִ� Ŭ��
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
		// 2. ������ ����
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
		// ��ġ�� �ٲ���ٰ� Ŭ�󿡰� �˷���

		for (auto& cl : clients) {
			if (cl._use == true)
				send_move_packet(cl._id, c_id);
		}
		break;



		// �̵��� Ÿ������ �Ǵ��Ѵ�

		// �̵��� Ÿ���ϰ� �̵��� ���״ٸ� �̵��� ������ ��� Ŭ�󿡰� ��������

	}break;
	case CS_PACKET_ATTACK: {
		for (auto& mon : monsters) {
			// ���� �÷��̾� ������ ������ ������ ���Ͱ� �ִ��� Ȯ��
			if ((mon.x <= cl.x + 1 && cl.x - 1 <= mon.x) &&
				(mon.y <= cl.y + 1 && cl.y - 1 <= mon.y)) {
				if(mon._live == true)
					send_combat_packet(c_id, mon._id, T_HUMAN);
			}
		}
	}break;
	}
}

// �ٸ� �����忡�� �۵�
void monster_ai()
{
	MONSTER& mon = monsters[0];
	while (1) {
		if (mon._live == false) {
			this_thread::sleep_for(chrono::seconds(3));
			mon.hp = 500000;
			mon.do_attack = false;

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

			// �̺κ� Datarace������
			for (auto& cl : clients) {
				if (cl._use == true)
					cl.do_send(sizeof(packet), &packet);
			}
			mon._live = true;
		}
		else {
			// ������ �÷��̾� �ִٸ� �÷��̾� ����
			if (mon.do_attack) {
				for (auto& cl : clients) {
					if (cl._use == true) {
						if ((cl.x <= mon.x + 1 && mon.x - 1 <= cl.x) &&
							(cl.y <= mon.y + 1 && mon.y - 1 <= cl.y)) {
							send_combat_packet(cl._id, mon._id, T_MONSTER);
						}
					}
				}
			}
			this_thread::sleep_for(chrono::seconds(1));
		}
	}
}

void worker()
{
	while (1) {
		DWORD num_byte;		// ���۹ްų� ���۵� �������� ��
		LONG64 iocp_key;	// �̸� ���� ���� ID
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		// ����ó�� (������ ������ ������ ���̰� ������ ���������� ó�� ���־�� �Ѵ�)
		if (ret == FALSE) {
			// int err_no = WSAGetLastError();
			// error_display(err_no);
			Disconnect(client_id);
			if (exp_over->_comp_op == OP_SEND)
				delete exp_over;			// �߸� �������� ������ ����
			continue;
		}

		switch (exp_over->_comp_op) {
		case OP_RECV: {
			CLIENT& cl = clients[client_id];
			int remain_data = num_byte + cl._prev_size;
			unsigned char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				// ��Ŷ ó��
				process_packet(client_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			// �������� ������ ó��
			if (0 < remain_data) {
				cl._prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}

			cl.do_recv();

		} break;

		case OP_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {	// �߰��� ©�� �����
				// DISCONNECT();
			}
			delete exp_over;
		} break;

		case OP_ACCEPT: {
			int new_id = get_new_id();
			CLIENT& cl = clients[new_id];

			// Ŭ���� ���� �ʱ�ȭ
			cl.x = 0;
			cl.y = 0;
			cl.level = 50;
			cl.hp = 54000;
			cl.mp = 27500;
			cl.physical_attack = 1250;
			cl.magical_attack = 500;
			cl.physical_defense = 1100;
			cl.magical_defense = 925;

			// ����� ���� ������ �ʱ�ȭ
			cl._id = new_id;
			cl._prev_size = 0;
			cl._recv_over._comp_op = OP_RECV;
			cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
			cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
			ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
			cl._sock = client_sock;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(cl._sock), h_iocp, new_id, 0);
			cl.do_recv();

			// Accept�� �Ϸᰡ �Ǿ����� �ٽ� accept�� ���־�� �Ѵ�
			ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
			client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(server_sock, client_sock, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
			break;
			} 
		}
	}
}

void do_timer()
{
	chrono::system_clock::duration dura;
	const chrono::milliseconds waittime = 10ms;
	timer_event temp;
	bool temp_bool = false;
	while (true) {
		timer_event ev;
		if (timer_queue.size() == 0) break;
		timer_queue.try_pop(ev);

		dura = ev.start_time - chrono::system_clock::now();
		if (dura <= 0ms) {
			EXP_OVER* ex_over = new EXP_OVER;
			//ex_over->_comp_op = EVtoOP(ev.ev);
			ex_over->_target = ev.target_id;
			PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);   //0�� ��������� ����
		}
		else if (dura <= waittime) {
				temp = ev;
				temp_bool = true;
				break;
		}
		else {
			timer_queue.push(ev);
		}
		this_thread::sleep_for(dura);
	}
}







int main()
{
	setlocale(LC_ALL, "korean");
	wcout.imbue(locale("korean"));

	// ���� �ʱ�ȭ
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// listen ���� ����
	server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind()
	bind(server_sock, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(server_sock, SOMAXCONN);

	// iocp �ڵ� ��ü ����
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// �ڵ鿡 ���� ����
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_sock), h_iocp, 0, 0);

	// AI�� ���� ������ �����
	thread monster_ai_thread(monster_ai);


	// Ŭ�� ���� ���� -> AcceptEx
	SOCKET client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	EXP_OVER accept_ex;

	ZeroMemory(&accept_ex, sizeof(accept_ex));
	accept_ex._comp_op = OP_ACCEPT;
	
	AcceptEx(server_sock, client_sock, accept_buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

	vector<thread> worker_threads;
	thread timer_thread{ do_timer };

	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(worker);

	for (auto& th : worker_threads)
		th.join();

	timer_thread.join();

	//���� �����忡 ���� join()
	monster_ai_thread.join();

	closesocket(server_sock);
	WSACleanup();
	return 0;
}
