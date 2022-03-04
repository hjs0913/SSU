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

	_look_x = 0.0f;
	_look_y = 0.0f;
	_look_z = 1.0f;

	_right_x = 1.0f;
	_right_y = 0.0f;
	_right_z = 0.0f;

	// _x = rand() % WORLD_WIDTH;
	// _y = rand() % WORLD_HEIGHT;
	_target_id = -1;
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

	_look_x = 0.0f;
	_look_y = 0.0f;
	_look_z = 1.0f;

	_right_x = 1.0f;
	_right_y = 0.0f;
	_right_z = 0.0f;

	_target_id = -1;
}

Npc::~Npc() 
{
}

void Npc::set_pos(int x, int z)
{
	_x = x;
	_z = z;
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

void Npc::set_mp(int mp)
{
	_mp = mp;
}

void Npc::set_maxmp(int m_mp)
{
	_maxmp = m_mp;
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

void Npc::set_basic_attack_factor(float basic_attack)
{
	_basic_attack_factor = basic_attack;
}

void Npc::set_defence_factor(float defence_factor)
{
	_defence_factor = defence_factor;
}

void Npc::set_look(float look_x, float look_y, float look_z)
{
	_look_x = look_x;
	_look_y = look_y;
	_look_z = look_z;
}

void Npc::set_right(float right_x, float right_y, float right_z)
{
	_right_x = right_x;
	_right_y = right_y;
	_right_z = right_z;
}

void Npc::set_mon_species(int s)
{
	_mon_species = static_cast<MONSTER_SPECIES>(s);
}


// get
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

int Npc::get_mp()
{
	return _mp;
}

int Npc::get_maxmp()
{
	return _maxmp;
}
float Npc::get_physical_attack()
{
	return _physical_attack;
}
float Npc::get_magical_attack()
{
	return _magical_attack;
}
float Npc::get_physical_defence()
{
	return _physical_defence;
}
float Npc::get_magical_defence()
{
	return _magical_defence;
}


float Npc::get_basic_attack_factor()
{
	return _basic_attack_factor;
}

float Npc::get_defence_factor()
{
	return _defence_factor;
}

float Npc::get_look_x()
{
	return _look_x;
}

float Npc::get_look_y()
{
	return _look_y;
}

float Npc::get_look_z()
{
	return _look_z;
}

float Npc::get_right_x()
{
	return _right_x;
}

float Npc::get_right_y()
{
	return _right_y;
}

float Npc::get_right_z()
{
	return _right_z;
}

MONSTER_SPECIES Npc::get_mon_spices()
{
	return _mon_species;
}

//skill
void Npc::set_skill_factor(int skill_type, int skill_num)
{

	switch (skill_type)
	{
	case 0:  //물리
		_skill_factors[skill_type][skill_num] = _lv * 0.5 * (skill_num + 1) + 50;
		break;
	case 1:  //마법
		_skill_factors[skill_type][skill_num] = _lv * 0.24 * (skill_num + 1) + 50;
		break;
	case 2:  // 버프 
		break;
	default:
		break;
	}


}
float Npc::get_skill_factor(int skill_type, int skill_num)
{
	return _skill_factors[skill_type][skill_num];
}

void  Npc::set_target_id(int target)
{
	_target_id = target;
}

int  Npc::get_target_id()
{
	return _target_id;
}

void Npc::set_element(ELEMENT element)
{
	_element = element;
}
ELEMENT Npc::get_element()
{
	return _element;
}
