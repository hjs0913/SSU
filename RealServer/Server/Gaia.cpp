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

	party_id = new int[GAIA_ROOM];

	running_pattern = false;

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
	party_id[player_cnt] = pl->get_id();
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
			send_start_gaia_packet(party[i], party_id);
			party[i]->indun_id = dungeon_id;

			// ���� ü���� ���� �÷��̾ �ϴ� Ÿ������ ��´�
			if (party[i]->get_hp() > tmp_hp) target_id = i;
		}

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
	if (running_pattern) return;

	if ((party[target_id]->get_x() >= boss->get_x() - 8 && party[target_id]->get_x() <= boss->get_x() + 8) &&
		(party[target_id]->get_z() >= boss->get_z() - 8 && party[target_id]->get_z() <= boss->get_z() + 8)) return;

	pos mv = boss->non_a_star(party[target_id]->get_x(), party[target_id]->get_z(), boss->get_x(), boss->get_z());
	// send boss position
	//if(mv == ������ �������� �ʴ°�) 
	//		���� �����Ű�� ���ο� ��ǥ�� Ŭ���̾�Ʈ���� �����ֱ�
	boss->set_x(mv.first);
	boss->set_z(mv.second);
	cout << mv.first << "," << mv.second << endl;
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
	timer_event ev;

	int p = pattern(gen);
	if (fifteen_pattern == false) {
		if (boss->get_hp() < boss->get_maxhp()/2) {
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
		// ��ǥ����
		// 1�� Ÿ��
		pattern_one_position[0].first = boss->get_x() + boss->get_look_x() * 50;
		pattern_one_position[0].second = boss->get_z() + boss->get_look_z() * 50;
		// 2�� Ÿ��
		pattern_one_position[1].first = boss->get_x() - boss->get_look_x() * 50;
		pattern_one_position[1].second = boss->get_z() - boss->get_look_z() * 50;
		// 3�� Ÿ��
		pattern_one_position[2].first = boss->get_x() + boss->get_right_x() * 50;
		pattern_one_position[2].second = boss->get_z() + boss->get_right_z() * 50;
		// 4�� Ÿ��
		pattern_one_position[3].first = boss->get_x() - boss->get_right_x() * 50;
		pattern_one_position[3].second = boss->get_z() - boss->get_right_z() * 50;

		for (int i = 0; i < GAIA_ROOM; i++) {
			send_gaia_pattern_one_packet(party[i], pattern_one_position);
		}
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 2s;
		ev.ev = EVENT_GAIA_PATTERN;
		ev.target_id = 0;
		timer_queue.push(ev);
		
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 5s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 1: {
		cout << "���� 1(���� ����)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 2: {
		cout << "���� 2(�� �ȱ�)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 3: {
		cout << "���� 3(������ ����)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 4: {
		cout << "���� 4(����)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
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

void Gaia::pattern_active(int pattern)
{
	switch (pattern) {
	case 0: {
		// ������ �ߵ��Ǿ��ٰ� Ŭ�� ��������
		
		// ���� ���� Ȯ��
		for (auto& p : party) {
			for (int i = 0; i < 4; i++) {
				int x = pattern_one_position[i].first;
				int z = pattern_one_position[i].second;

				if (sqrt((p->get_x() - x) * (p->get_x() - x) + (p->get_z() - z) * (p->get_z() - z)) < 20) {
					p->set_hp(p->get_hp() - 2000);
					// ���� ��������� ������ �¾Ҵٰ� ��������
					for (auto send_pl : party) {
						send_change_hp_packet(send_pl, p);
					}
				}
			}
			send_gaia_pattern_one_active_packet(p);
		}

		break;
	}
	default:
		cout << "�ߺ��� ���� Ȱ��ȭ" << endl;
		break;
	}
}
