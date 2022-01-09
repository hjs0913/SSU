#include "Npc.h"

Npc::Npc(int id)
{
	_id = id;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;

	_x = 300;
	_y = 0;
	_z = 300;

	// _x = rand() % WORLD_WIDTH;
	// _y = rand() % WORLD_HEIGHT;
}

Npc::Npc(int id, const char* name)
{
	sprintf_s(_name, MAX_NAME_SIZE, name);
	_id = id;
	_x = rand() % WORLD_WIDTH;
	_y = rand() % WORLD_HEIGHT;
	_lv = 1;
	_hp = 100;
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

void Npc::set_maxhp(int m_hp)
{
	_maxhp = m_hp;
}

void Npc::set_physical_attack(float physical_attack)
{
	_physical_attack = physical_attack;
}

void Npc::set_magical_attack(float magical_attack)
{
	_magical_attack = magical_attack;
}

void Npc::set_physical_defence(float physical_defence)
{
	_physical_defence = physical_defence;
}

void Npc::set_magical_defence(float magical_defence)
{
	_magical_defence = magical_defence;
}

// get

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

int Npc::get_maxhp()
{
	return _maxhp;
}