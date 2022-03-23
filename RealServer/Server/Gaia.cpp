#include "Gaia.h"

Gaia::Gaia(int d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_ROBBY;
	player_cnt = 0;
	// Boss Npc Intialize	

}

Gaia::~Gaia()
{
	delete party;
}

void Gaia::join_player(Player* pl)
{
	party[player_cnt] = pl;
	player_cnt++;
	cout << dungeon_id << "번 던전에 입장 중입니다" << endl;
	pl->state_lock.lock();
	pl->join_dungeon_room = true;
	pl->state_lock.unlock();
	// game start
	if (player_cnt == 4) {
		// 모든 파티 인던 입장 및 게임 시작
		
		for (auto pt : party) {
			pt->state_lock.lock();
			pt->set_state(ST_INDUN);
			pt->state_lock.unlock();
		}

		state_lock.lock();
		st = DUN_ST_START;
		state_lock.unlock();
		cout << dungeon_id << "번 던전 시작합니다" << endl;
	}
}

DUNGEON_STATE Gaia::get_dun_st()
{
	return st;
}

Player** Gaia::get_party_palyer()
{
	return party;
}