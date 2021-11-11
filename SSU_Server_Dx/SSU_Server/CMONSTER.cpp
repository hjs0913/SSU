#include "CMONSTER.h"

MONSTER::MONSTER() :tribe(T_MONSTER)
{
	_live = true;
	_id = 0;

	strcpy_s(name, sizeof(name), "Monster");
	// 일단 임시로 적어놈
	x = 6;
	y = 6;
	level = 50;
	hp = 500000;
	physical_attack = 750;
	physical_defense = 1200;
	magical_defense = 500;
	attack_factor = 10;
	defense_factor = 0.0002;
	element = E_ICE;
	tribe = T_MONSTER;
	do_attack = false;
}

MONSTER::~MONSTER() {};
