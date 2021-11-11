#include "CCLIENT.h"

EXP_OVER::EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
{
	ZeroMemory(&_wsa_over, sizeof(_wsa_over));
	_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
	_wsa_buf.len = num_bytes;
	memcpy(_net_buf, mess, num_bytes);
};

EXP_OVER::EXP_OVER(COMP_OP comp_op) : _comp_op(comp_op) {};

EXP_OVER::EXP_OVER() { _comp_op = OP_RECV; };

EXP_OVER::~EXP_OVER() {};

CLIENT::CLIENT() : tribe(T_HUMAN)
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
	magical_defense = 925;
	attack_factor = 50;
	defense_factor = 0.0002;
	exp = 0;
	element = E_FULLMETAL;
}

CLIENT::~CLIENT() { closesocket(_sock); };

void CLIENT::do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
	_recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
	_recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
	int ret = WSARecv(_sock, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (ERROR_IO_PENDING != err_num) {
			//Disconnect(_id);
		}
	}
}

void CLIENT::do_send(int num_bytes, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_bytes, mess);
	WSASend(_sock, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
}
