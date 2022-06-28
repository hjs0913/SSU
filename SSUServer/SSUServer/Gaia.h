#pragma once
#include "Player.h"

class Gaia
{
private:
// 기본정보

	int dungeon_id;
	DUNGEON_STATE st;
	char room_name[MAX_NAME_SIZE];
	Player* party[GAIA_ROOM];	// 파티원 정보
	int party_id[GAIA_ROOM] = { -1, -1, -1, -1 };	//서버에서의 파티원 id
	int  player_death_count = 4;

// 패턴 정보
	chrono::system_clock::time_point start_time;

public:	
	int target_id;	// agro_id (이때 id는 파티원에게 부여된 파티내 아이디(0~GAIA_ROOM)
	bool running_pattern;  //파트너가 알수있게 퍼블릭으로
	int pattern_num;   
	// 1번
	pos pattern_one_position[4];
	// 2번
	pos pattern_two_position[3];
	int pattern_two_number;
	int pattern_two_count = 0;
	pos pattern_two_safe_zone[4];

	// 5번
	pos pattern_five_position[2];
	int pattern_five_count = 0;
	// 특수
	bool fifteen_pattern;


	int player_cnt;
	Npc* boss;
	mutex state_lock;
	int player_rander_ok;
	bool start_game;
	int partner_cnt;



public:
	Gaia(int d_id);
	~Gaia();

	void join_player(Player* pl);
	void quit_palyer(Player* pl);

	void game_start();
	void game_victory();
	void destroy_dungeon();

	DUNGEON_STATE get_dun_st();
	void set_dun_st(DUNGEON_STATE dst);
	
	Player** get_party_palyer();
	char* get_party_name();
	void set_party_name(char* name);

	void boss_move();
	void boss_attack();

	int get_dungeon_id();

	void pattern_active(int pattern);

	bool check_inside(pos a, pos b, pos c, pos n);
	bool isInsideTriangle(pos a, pos b, pos c, pos n);

	void judge_pattern_two_rightup(Player* p, int pattern_number);
	void judge_pattern_two_leftup(Player* p, int pattern_number);

	void player_death(Player* p);
	
	float get_x();
	float get_z();
};

