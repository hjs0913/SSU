#pragma once
#include "Player.h"

class Gaia
{
private:
// 기본정보
	Player* party[GAIA_ROOM];	// 파티원 정보
	int dungeon_id;
	int player_cnt;
	DUNGEON_STATE st;

	int target_id;	// agro_id (이때 id는 파티원에게 부여된 파티내 아이디(0~GAIA_ROOM)

	int* party_id;	//서버에서의 파티원 id
	int  player_death_count = 4;

// 패턴 정보
	bool running_pattern;
	// 1번
	pos pattern_one_position[4];
	// 2번
	pos pattern_two_position[3];
	int pattern_two_number;
	int pattern_two_count = 0;
	// 5번
	pos pattern_five_position[2];
	int pattern_five_count = 0;
	// 특수
	bool fifteen_pattern;



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

	bool check_inside(pos a, pos b, pos c, pos n);
	bool isInsideTriangle(pos a, pos b, pos c, pos n);

	void judge_pattern_two_rightup(Player* p);
	void judge_pattern_two_leftup(Player* p);

	void player_death(Player* p);
};

