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
	int target_id;

	int* party_id;

	pos pattern_one_position[4];

	bool running_pattern;

public:
	Npc* boss;
	mutex state_lock;
	int player_rander_ok;
	bool start_game;

public:
	Gaia(int d_id);
	~Gaia();

	void join_player(Player* pl);
	DUNGEON_STATE get_dun_st();
	Player** get_party_palyer();

	void boss_move();
	void boss_attack();

	int get_dungeon_id();

	void pattern_active(int pattern);
};

