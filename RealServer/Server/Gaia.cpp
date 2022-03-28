#include "Gaia.h"
#include "send.h"
#include <random>

Gaia::Gaia(int d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_ROBBY;
	player_cnt = 0;

	fifteen_pattern = false;

	// Boss Npc Intialize	
	boss = new Npc(dungeon_id);
	boss->set_tribe(BOSS);

	boss->state_lock.lock();
	boss->set_state(ST_FREE);
	boss->state_lock.unlock();

	lua_State* L = boss->L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L, "Raid_Gaia.lua") || lua_pcall(L, 0, 0, 0);
	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, dungeon_id);
	error = lua_pcall(L, 1, 10, 0);

	boss->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
	boss->set_lv(lua_tointeger(L, -9));

	boss->set_name(lua_tostring(L, -8));

	boss->set_hp(lua_tointeger(L, -7));
	boss->set_maxhp(lua_tointeger(L, -7));

	boss->set_physical_attack(lua_tonumber(L, -6));
	boss->set_magical_attack(lua_tonumber(L, -5));
	boss->set_physical_defence(lua_tonumber(L, -4));
	boss->set_magical_defence(lua_tonumber(L, -3));
	boss->set_basic_attack_factor(lua_tointeger(L, -2));
	boss->set_defence_factor(lua_tonumber(L, -1));

	lua_pop(L, 11);// eliminate set_uid from stack after call

	boss->set_x(310);
	boss->set_x(110);

	// 나중에 어떻게 이용할 것인지 생각
	// lua_register

	boss->set_mon_species(RAID_GAIA);
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
	if (player_cnt == GAIA_ROOM) {
		// 모든 파티 인던 입장 및 게임 시작
		state_lock.lock();
		st = DUN_ST_START;
		state_lock.unlock();
		
		boss->set_x(party[0]->get_x() + 10);
		boss->set_z(party[0]->get_z() + 10);

		for (auto pt : party) {
			pt->state_lock.lock();
			pt->set_state(ST_INDUN);
			pt->state_lock.unlock();
			send_start_gaia_packet(pt);
			pt->indun_id = dungeon_id;
			// 모든 좌표 및 정보들을 초기화 하자
			pt->set_pos(300, 100);
		}

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

void Gaia::boss_move()
{
	// 프레임 워크 수정하고
	// target_id에 맞게 a_star로 쫒아간다
	cout << "원래는 쫒아가는 중" << endl;
}

void Gaia::boss_attack()
{
	default_random_engine dre;
	uniform_int_distribution<int> pattern(0, 4);

	int p = pattern(dre);
	if (fifteen_pattern == false) {
		if (boss->get_hp() < boss->get_maxhp()) {
			fifteen_pattern = true;
			cout << "특수 패턴을 실행합니다" << endl;
			return;
		}
	}
	else {
		if (boss->get_hp() <= 0) {
			cout << "마지막 발악을 실행합니다" << endl;
			return;
		}
	}

	switch (p) {
	case 0: {
		cout << "패턴 0(대지 흔들기)" << endl;
		break;
	}
	case 1: {
		cout << "패턴 1(대지 해일)" << endl;
		break;
	}
	case 2: {
		cout << "패턴 2(검 꽂기)" << endl;
		break;
	}
	case 3: {
		cout << "패턴 3(나뭇잎 공격)" << endl;
		break;
	}
	case 4: {
		cout << "패턴 4(참격)" << endl;
		break;
	}
	default:
		cout << "패턴 에러" << endl;
		break;
	}
}