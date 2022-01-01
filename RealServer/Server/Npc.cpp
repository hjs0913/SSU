#include "Npc.h"

Npc::Npc(int id)
{
	_id = id;
	_x = rand() % WORLD_WIDTH;
	_y = rand() % WORLD_HEIGHT;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;
}

Npc::Npc(int id, const char* name)
{
	sprintf_s(_name, MAX_NAME_SIZE, name);
	_id = id;
	_x = rand() % WORLD_WIDTH;
	_y = rand() % WORLD_HEIGHT;
	_lv = 1;
	_hp = 100;
	_exp = 0;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;
}

Npc::~Npc() 
{
}

void Npc::set_pos(int x, int y)
{
	_x = x;
	_y = y;
}

void Npc::set_x(int x)
{
	_x = x;
}

void Npc::set_y(int y)
{
	_y = y;
}

void Npc::set_state(STATE s)
{
	_state = s;
}

void Npc::set_name(const char* name)
{
	strncpy_s(_name, name, sizeof(_name));
}

void Npc::set_active(bool act)
{
	_active = act;
}

void Npc::set_id(int id)
{
	_id = id;
}

void Npc::set_tribe(TRIBE tribe)
{
	_tribe = tribe;
}

void Npc::set_lv(short lv)
{
	_lv = lv;
}

void Npc::set_hp(int hp)
{
	_hp = hp;
}

void Npc::set_exp(int exp)
{
	_exp = exp;
}

void Npc::set_maxhp(int m_hp)
{
	_maxhp = m_hp;
}

int Npc::get_x()
{
	return _x;
}

int Npc::get_y()
{
	return _y;
}

int Npc::get_Id()
{
	return _id;
}

char* Npc::get_name()
{
	return _name;
}

STATE Npc::get_state()
{
	return _state;
}

bool Npc::get_active()
{
	return _active;
}

TRIBE Npc::get_tribe()
{
	return _tribe;
}

short Npc::get_lv()
{
	return _lv;
}

int Npc::get_hp()
{
	return _hp;
}

int Npc::get_exp()
{
	return _exp;
}

int Npc::get_maxhp()
{
	return _maxhp;
}