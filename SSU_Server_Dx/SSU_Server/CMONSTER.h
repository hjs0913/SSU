#pragma once
#include <iostream>
#include "CBUFF.h"

using namespace std;

class MONSTER
{
public:
	volatile bool _live;

	int _id;
	char name[MAX_ID_LEN];
	int x, y, z;
	int hp;
	int physical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int attack_factor;
	float defense_factor;
	TRIBE tribe;
	bool do_attack;
	BUFF buff_element;
	NUFF nuff_element;

public:
	MONSTER();

	~MONSTER();

};

