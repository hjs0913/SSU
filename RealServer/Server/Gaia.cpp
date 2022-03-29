#include "Gaia.h"
#include "send.h"
#include <random>
#include <ctime>

Gaia::Gaia(int d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_ROBBY;
	player_cnt = 0;

	fifteen_pattern = false;

	player_rander_ok = 0;
	target_id = 0;
	start_game = false;

	// Boss Npc Intialize	
	boss = new Npc(dungeon_id);
	boss->set_tribe(BOSS);
	boss->state_lock.lock();
	boss->set_state(ST_FREE);
	boss->state_lock.unlock();
	boss->set_id(101);

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

	// ���߿� ��� �̿��� ������ ����
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
	cout << dungeon_id << "�� ������ ���� ���Դϴ�" << endl;
	pl->state_lock.lock();
	pl->join_dungeon_room = true;
	pl->state_lock.unlock();
	// game start
	if (player_cnt == GAIA_ROOM) {
		// ��� ��Ƽ �δ� ���� �� ���� ����
		player_rander_ok = 0;
		// ���� ü���� ���� �÷��̾ �ϴ� Ÿ������ ��´�
		int tmp_hp = 0;

		state_lock.lock();
		st = DUN_ST_START;
		state_lock.unlock();
		
		boss->set_x(party[0]->get_x() + 10);
		boss->set_z(party[0]->get_z() + 10);

		for (int i = 0; i < GAIA_ROOM; i++) {
			party[i]->state_lock.lock();
			party[i]->set_state(ST_INDUN);
			party[i]->state_lock.unlock();
			send_start_gaia_packet(party[i]);
			party[i]->indun_id = dungeon_id;

			// ���� ü���� ���� �÷��̾ �ϴ� Ÿ������ ��´�
			if (party[i]->get_hp() > tmp_hp) target_id = i;

			// ��� ��ǥ �� �������� �ʱ�ȭ ����
			//party[i]->set_pos(300, 100);
		}

		//for (auto pt : party) {
		//	pt->state_lock.lock();
		//	pt->set_state(ST_INDUN);
		//	pt->state_lock.unlock();
		//	send_start_gaia_packet(pt);
		//	pt->indun_id = dungeon_id;

		//	// ���� ü���� ���� �÷��̾ �ϴ� Ÿ������ ��´�
		//	if (pt->get_hp() > tmp_hp) boss->set_target_id(pt->get_id());

		//	// ��� ��ǥ �� �������� �ʱ�ȭ ����
		//	pt->set_pos(300, 100);
		//}

		cout << dungeon_id << "�� ���� �����մϴ�" << endl;
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
	// Raid Map�� ��ֹ��� �����Ƿ� A_star�� �����

	if ((party[target_id]->get_x() >= boss->get_x() - 8 && party[target_id]->get_x() <= boss->get_x() + 8) &&
		(party[target_id]->get_z() >= boss->get_z() - 8 && party[target_id]->get_z() <= boss->get_z() + 8)) return;

	pos mv = boss->non_a_star(party[target_id]->get_x(), party[target_id]->get_z(), boss->get_x(), boss->get_z());
	// send boss position
	//if(mv == ������ �������� �ʴ°�) 
	//		���� �����Ű�� ���ο� ��ǥ�� Ŭ���̾�Ʈ���� �����ֱ�
	cout << "���� �����̴� �� : " << mv.first << "," << mv.second << endl;
	boss->set_x(mv.first);
	boss->set_z(mv.second);
	for (auto pt : party) {
		send_move_packet(pt, boss);
		send_look_packet(pt, boss);
	}
	
}

void Gaia::boss_attack()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);

	int p = pattern(gen);
	if (fifteen_pattern == false) {
		if (boss->get_hp() < boss->get_maxhp()) {
			fifteen_pattern = true;
			cout << "Ư�� ������ �����մϴ�" << endl;
			return;
		}
	}
	else {
		if (boss->get_hp() <= 0) {
			cout << "������ �߾��� �����մϴ�" << endl;
			return;
		}
	}
	cout << "p : " << p << endl;
	switch (p%5) {
	case 0: {
		cout << "���� 0(���� ����)" << endl;
		break;
	}
	case 1: {
		cout << "���� 1(���� ����)" << endl;
		break;
	}
	case 2: {
		cout << "���� 2(�� �ȱ�)" << endl;
		break;
	}
	case 3: {
		cout << "���� 3(������ ����)" << endl;
		break;
	}
	case 4: {
		cout << "���� 4(����)" << endl;
		break;
	}
	default:
		cout << "���� ����" << endl;
		break;
	}
}

int Gaia::get_dungeon_id()
{
	return dungeon_id;
}
