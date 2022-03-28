#pragma once
#include "Player.h"

class Gaia
{
private:
	Player* party[GAIA_ROOM];
	int dungeon_id;
	int player_cnt;
	DUNGEON_STATE st;

	bool fifteen_pattern;

public:
	Npc* boss;
	mutex state_lock;

public:
	Gaia(int d_id);
	~Gaia();

	void join_player(Player* pl);
	DUNGEON_STATE get_dun_st();
	Player** get_party_palyer();

	void boss_move();
	void boss_attack();
};

