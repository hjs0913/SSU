#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "CBUFF.h"
#include "stdafx.h"
#include <unordered_set>
#include <mutex>

enum STATE { ST_FREE, ST_ACCEPT, ST_INGAME };
class EXP_OVER
{
public:
	WSAOVERLAPPED	_wsa_over;
	WSABUF			_wsa_buf;
	unsigned char	_net_buf[BUFSIZE];
	COMP_OP			_comp_op;

public:
	EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess);

	EXP_OVER(COMP_OP comp_op);

	EXP_OVER();		// array에 넣기 위해서는 기본생성자가 있어야한다

	~EXP_OVER();
};

class CLIENT
{
public:
	EXP_OVER _recv_over;
	SOCKET	_sock;
	int _prev_size;
	int _id;

	bool _use;
	bool _live;

	char name[MAX_ID_LEN];
	float x, y, z;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int exp;
	short attack_factor;
	float defense_factor;
	TRIBE tribe;
	BUFF buff_element;
	NUFF nuff_element;

	mutex vl;
	mutex state_lock;

	STATE _state;

	unordered_set <int> viewlist;
public:
	CLIENT();
	
	//CLIENT();
	~CLIENT();
	
	void do_recv();

	void do_send(int num_bytes, void* mess);
};