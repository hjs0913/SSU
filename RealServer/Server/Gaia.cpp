#include "Gaia.h"
#include "send.h"
#include <random>
#include <ctime>

Gaia::Gaia(int d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_FREE;
	player_cnt = 0;

	fifteen_pattern = false;

	player_rander_ok = 0;
	target_id = 0;
	start_game = false;
	partner_cnt = 0;
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

	// 나중에 어떻게 이용할 것인지 생각
	// lua_register

	boss->set_mon_species(RAID_GAIA);
}

Gaia::~Gaia()
{
	delete party;
}
float Gaia::get_x()
{
	return boss->get_x();
}

float Gaia::get_z()
{
	return boss->get_z();
}

void Gaia::join_player(Player* pl)
{

	party[player_cnt] = pl;
	party_id[player_cnt] = pl->get_id();
	player_cnt++;
	cout << "player_cnt : " << player_cnt << endl;
	cout << dungeon_id << "번 던전에 입장 중입니다" << endl;

	pl->set_indun_id(dungeon_id);
	pl->state_lock.lock();
	pl->join_dungeon_room = true;
	pl->state_lock.unlock();

	//send_gaia_join_ok(pl, dungeon_id);

	// game start
	if (player_cnt == GAIA_ROOM) {
		// 여기에 인원이 꽉찼으면 5초후 게임을 시작하는 타이머를 돌려주자
		
		timer_event ev;
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 5s;
		ev.ev = EVENT_GAMESTART_TIMER;
		ev.target_id = dungeon_id;
		timer_queue.push(ev);

		start_time = ev.start_time;
		
		// 5초후 게임을 시작한다는 패킷을 보내주자
		for (int i = 0; i < GAIA_ROOM; i++) {
			if (party[i]->get_tribe() != HUMAN) continue;
			char notice_str[MAX_CHAT_SIZE];
			strcpy_s(notice_str, "5초후에 게임을 시작합니다");
			send_notice(party[i], notice_str, 0);
		}
	}
}

void Gaia::quit_palyer(Player* pl)
{
	int tmp = 0;
	for (int i = 0; i < GAIA_ROOM; i++) {
		if (party_id[i] == pl->get_id()) tmp = i;
	}

	// 앞으로 다 땡겨오기
	for (int i = tmp; i < GAIA_ROOM; i++) {
		if (i == GAIA_ROOM - 1) continue;
		party[i] = party[i + 1];
		party_id[i] = party_id[i + 1];
	}



	pl->set_indun_id(-1);
	pl->state_lock.lock();
	pl->join_dungeon_room = false;
	pl->state_lock.unlock();
	player_cnt--;
}

void Gaia::game_start()
{
	// 게임을 시작할 수 있는 조건인지 확인
	if (start_time > chrono::system_clock::now()) return;
	if (player_cnt < GAIA_ROOM) return;

	// AI의 정보를 클라이언트에게 보내주어야한다
	for (int i = 0; i < GAIA_ROOM; i++) {
		if (party[i]->get_tribe() == PARTNER) {
			for (int j = 0; j < GAIA_ROOM; j++) {
				if(party[j]->get_tribe() == HUMAN) send_put_object_packet(party[j], party[i]);
			}
		}
	}


	// 모든 파티 인던 입장 및 게임 시작
	player_rander_ok = 0;
	// 가장 체력이 높은 플레이어를 일단 타겟으로 잡는다
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
		if(party[i]->get_tribe() == HUMAN)
			send_start_gaia_packet(party[i], party_id);
		party[i]->indun_id = dungeon_id;
		// 가장 체력이 높은 플레이어를 일단 타겟으로 잡는다
		if (party[i]->get_hp() > tmp_hp) target_id = 0;
	}

	cout << dungeon_id << "번 던전 시작합니다" << endl;
}

void Gaia::destroy_dungeon()
{
	state_lock.lock();
	set_dun_st(DUN_ST_FREE);
	state_lock.unlock();
	
	player_cnt = 0;
	partner_cnt = 0;

	fifteen_pattern = false;

	player_rander_ok = 0;
	target_id = 0;
	start_game = false;
	running_pattern = false;

	ZeroMemory(party, sizeof(Player*)*4);
	ZeroMemory(party_id, sizeof(int) * 4);
	player_death_count = 4;

	// Boss Npc Intialize	
	boss->state_lock.lock();
	boss->set_state(ST_FREE);
	boss->state_lock.unlock();
	boss->set_id(101);

	// 루아를 이용한 초기화
	lua_State* L = boss->L;
	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, dungeon_id);
	int error = lua_pcall(L, 1, 10, 0);
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
}

DUNGEON_STATE Gaia::get_dun_st()
{
	return st;
}

void Gaia::set_dun_st(DUNGEON_STATE dst)
{
	st = dst;
}

Player** Gaia::get_party_palyer()
{
	return party;
}

char* Gaia::get_party_name()
{
	return room_name;
}

void Gaia::set_party_name(char* name)
{
	strcpy_s(room_name, name);
}

void Gaia::boss_move()
{
	// Raid Map은 장애물이 없으므로 A_star는 낭비다
	if (running_pattern) return;

	if ((party[target_id]->get_x() >= boss->get_x() - 8 && party[target_id]->get_x() <= boss->get_x() + 8) &&
		(party[target_id]->get_z() >= boss->get_z() - 8 && party[target_id]->get_z() <= boss->get_z() + 8)) return;

	pos mv = boss->non_a_star(party[target_id]->get_x(), party[target_id]->get_z(), boss->get_x(), boss->get_z());
	// send boss position
	//if(mv == 밖으로 떨어지지 않는가) 
	//		값을 적용시키고 새로운 좌표를 클라이언트에게 보내주기
	boss->set_x(mv.first);
	boss->set_z(mv.second);
	for (auto pt : party) {
		if (pt->get_tribe() != HUMAN) continue;
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

	pattern_num = pattern(gen) % 5;

	if (fifteen_pattern == false) {
		if (boss->get_hp() < boss->get_maxhp()/2) {
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

	switch (pattern_num) {
	case 0: {  //장판
		cout << "장판" << endl;
		running_pattern = true;
		// 목표지점
		// 1차 타점
		pattern_one_position[0].first = boss->get_x() + boss->get_look_x() * 50;
		pattern_one_position[0].second = boss->get_z() + boss->get_look_z() * 50;
		// 2차 타점
		pattern_one_position[1].first = boss->get_x() - boss->get_look_x() * 50;
		pattern_one_position[1].second = boss->get_z() - boss->get_look_z() * 50;
		// 3차 타점
		pattern_one_position[2].first = boss->get_x() + boss->get_right_x() * 50;
		pattern_one_position[2].second = boss->get_z() + boss->get_right_z() * 50;
		// 4차 타점
		pattern_one_position[3].first = boss->get_x() - boss->get_right_x() * 50;
		pattern_one_position[3].second = boss->get_z() - boss->get_right_z() * 50;

		for (int i = 0; i < GAIA_ROOM; i++) {
			if (party[i]->get_tribe() != HUMAN) continue;
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
	case 1: { //파도3개 
		cout << "파도" << endl;
		running_pattern = true;
		pattern_two_number = pattern(gen) % 4;
		switch (pattern_two_number) {
		case 0: {
			pattern_two_position[0].first = 2172;
			pattern_two_position[0].second = 2462;

			pattern_two_position[1].first = 2272;
			pattern_two_position[1].second = 2362;

			pattern_two_position[2].first = 2372;
			pattern_two_position[2].second = 2262;

			pattern_two_safe_zone[0].first = 2222;  //내부 
			pattern_two_safe_zone[0].second = 2412;
			pattern_two_safe_zone[1].first = 2322;
			pattern_two_safe_zone[1].second = 2312;

			pattern_two_safe_zone[2].first = 2122;  //외부
			pattern_two_safe_zone[2].second = 2512;
			pattern_two_safe_zone[3].first = 2422;
			pattern_two_safe_zone[3].second = 2212;


			break;
		}
		case 1: {
			pattern_two_position[0].first = 1688;
			pattern_two_position[0].second = 2262;

			pattern_two_position[1].first = 1788;
			pattern_two_position[1].second = 2362;

			pattern_two_position[2].first = 1888;
			pattern_two_position[2].second = 2462;

			pattern_two_safe_zone[0].first = 1738;
			pattern_two_safe_zone[0].second = 2312;

			pattern_two_safe_zone[1].first = 1838;
			pattern_two_safe_zone[1].second = 2412;

			pattern_two_safe_zone[2].first = 1638;
			pattern_two_safe_zone[2].second = 2212;
			pattern_two_safe_zone[3].first = 1938;
			pattern_two_safe_zone[3].second = 2512;


			break;
		}
		case 2: {
			pattern_two_position[0].first = 1888;
			pattern_two_position[0].second = 1778;

			pattern_two_position[1].first = 1788;
			pattern_two_position[1].second = 1878;

			pattern_two_position[2].first = 1688;
			pattern_two_position[2].second = 1978;

			pattern_two_safe_zone[0].first = 1838;
			pattern_two_safe_zone[0].second = 1828;
			pattern_two_safe_zone[1].first = 1738;
			pattern_two_safe_zone[1].second = 1928;

			pattern_two_safe_zone[2].first = 1938;
			pattern_two_safe_zone[2].second = 1728;
			pattern_two_safe_zone[3].first = 1638;
			pattern_two_safe_zone[3].second = 2028;
			break;
		}
		case 3: {
			pattern_two_position[0].first = 2372;
			pattern_two_position[0].second = 1978;

			pattern_two_position[1].first = 2272;
			pattern_two_position[1].second = 1878;

			pattern_two_position[2].first = 2172;
			pattern_two_position[2].second = 1778;

			pattern_two_safe_zone[0].first = 2322;
			pattern_two_safe_zone[0].second = 1928;
			pattern_two_safe_zone[1].first = 2222;
			pattern_two_safe_zone[1].second = 1828;

			pattern_two_safe_zone[2].first = 2422;
			pattern_two_safe_zone[2].second = 2028;
			pattern_two_safe_zone[3].first = 2122;
			pattern_two_safe_zone[3].second = 1728;
			break;
		}
		}

		for (int i = 0; i < GAIA_ROOM; i++) {
			if (party[i]->get_tribe() != HUMAN) continue;
			send_gaia_pattern_two_packet(party[i], pattern_two_position, pattern_two_number);
		}
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 500ms;
		ev.ev = EVENT_GAIA_PATTERN;
		ev.target_id = 1;
		timer_queue.push(ev);
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 13s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 2: {
		cout << "패턴 2(검 꽂기)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 3: {
		cout << "패턴 3(나뭇잎 공격)" << endl;
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 4: {//참격 1개 
		cout << "참격" << endl;
		running_pattern = true;
		pattern_five_position[0] = pos(boss->get_x()+ boss->get_look_x(), boss->get_z() + boss->get_look_z());
		for (int i = 0; i < GAIA_ROOM; i++) {
			if (party[i]->get_tribe() != HUMAN) continue;
			send_gaia_pattern_five_packet(party[i], &pattern_five_position[0]);
		}
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 100ms;
		ev.ev = EVENT_GAIA_PATTERN;
		ev.target_id = 4;
		timer_queue.push(ev);
		//
		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 6s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	default:
		cout << "패턴 에러" << endl;
		break;
	}
}

int Gaia::get_dungeon_id()
{
	return dungeon_id;
}

void Gaia::player_death(Player* p)
{
	p->set_hp(0);
	std::cout << p->get_name() << "사망" << std::endl;
	if (player_death_count > 0) {
		player_death_count--;
		p->state_lock.lock();
		if (p->get_state() != ST_INDUN) {
			p->state_lock.unlock();
			return;
		}
		p->set_state(ST_DEAD);
		p->state_lock.unlock();

		// 시야처리
		int tmp_hp = 0;
		for (int i = 0; i < GAIA_ROOM; i++) {
			send_change_hp_packet(party[i], p);
			send_change_death_count_packet(party[i], player_death_count);
			if (p->get_id() != party[i]->get_id()) {
				send_remove_object_packet(party[i], p);
			}
			if (party[i]->get_hp() > tmp_hp) target_id = i;
		}

		send_notice(p, "사망했습니다. 5초 후 부활합니다", 1);

		timer_event ev;
		ev.obj_id = p->get_id();
		ev.start_time = chrono::system_clock::now() + 5s;
		ev.ev = EVENT_PLAYER_REVIVE;
		ev.target_id = 0;
		timer_queue.push(ev);
	}
	else {
		for (auto send_pl : party) {
			send_remove_object_packet(send_pl, p);
		}
	}
}

bool Gaia::check_inside(pos a, pos b, pos c, pos n) {
	pos A, B, C;
	A.first = b.first - a.first;
	A.second = b.second - a.second;
	B.first = c.first - a.first;
	B.second = c.second - a.second;
	C.first = n.first - a.first;
	C.second = n.second - a.second;

	if ((A.first * B.second - A.second * B.first) * (A.first * C.second - A.second * C.first) < 0)
		return false;
	return true;
}

bool Gaia::isInsideTriangle(pos a, pos b, pos c, pos n)
{
	if (!check_inside(a, b, c, n)) return false;
	if (!check_inside(b, c, a, n)) return false;
	if (!check_inside(c, a, b, n)) return false;
	return true;

}

void Gaia::judge_pattern_two_rightup(Player* p)
{
	for (int i = 0; i < 3; i++) {
		int x = pattern_two_position[i].first;
		int z = pattern_two_position[i].second;
		pos rect[4];
		rect[0] = pos(x - 35, z + 35);
		rect[1] = pos(x - 70, z);
		rect[2] = pos(x, z - 70);
		rect[3] = pos(x + 35, z - 35);
		pos n = pos(p->get_x(), p->get_z());

		p->state_lock.lock();
		if (p->get_state() != ST_INDUN) {
			p->state_lock.unlock();
			continue;
		}
		p->state_lock.unlock();

		if (isInsideTriangle(rect[0], rect[1], rect[2], n) || isInsideTriangle(rect[0], rect[2], rect[3], n)) {
			// 쳐 맞는 판정
			p->set_hp(p->get_hp() - 3000);
			if (p->get_hp() < 0) player_death(p);
			for (auto send_pl : party) {
				send_change_hp_packet(send_pl, p);
			}
		}
	}
}

void Gaia::judge_pattern_two_leftup(Player* p)
{
	for (int i = 0; i < 3; i++) {
		int x = pattern_two_position[i].first;
		int z = pattern_two_position[i].second;
		pos rect[4];
		rect[0] = pos(x - 35, z - 35);
		rect[1] = pos(x, z - 70);
		rect[2] = pos(x + 70, z);
		rect[3] = pos(x + 35, z + 35);
		pos n = pos(p->get_x(), p->get_z());

		p->state_lock.lock();
		if (p->get_state() != ST_INDUN) {
			p->state_lock.unlock();
			continue;
		}
		p->state_lock.unlock();

		if (isInsideTriangle(rect[0], rect[1], rect[2], n) || isInsideTriangle(rect[0], rect[2], rect[3], n)) {
			// 쳐 맞는 판정
			p->set_hp(p->get_hp() - 3000);
			if (p->get_hp() < 0) player_death(p);
			for (auto send_pl : party) {
				send_change_hp_packet(send_pl, p);
			}
		}
	}
}

void Gaia::pattern_active(int pattern)
{
	switch (pattern) {
	case 0: {

		// 패턴이 발동되었다고 클라에 보내주자

		// 패턴 판정 확인
		for (auto& p : party) {
			for (int i = 0; i < 4; i++) {
				int x = pattern_one_position[i].first;
				int z = pattern_one_position[i].second;

				p->state_lock.lock();
				if (p->get_state() != ST_INDUN) {
					p->state_lock.unlock();
					continue;
				}
				p->state_lock.unlock();

				if (sqrt((p->get_x() - x) * (p->get_x() - x) + (p->get_z() - z) * (p->get_z() - z)) < 20) {
					p->set_hp(p->get_hp() - 2000);
					if (p->get_hp() < 0) player_death(p);
					// 패턴 맞은사람이 있으면 맞았다고 보내주자
					for (auto send_pl : party) {
						send_change_hp_packet(send_pl, p);
					}
				}
			}
			send_gaia_pattern_finish_packet(p, 0);
			running_pattern = false;
		}

		break;
	}
	case 1: {
		float movesize = 25.0f * (float)(sqrt(2) / 2);
		for (auto& p : party) {
			if (pattern_two_number == 0 || pattern_two_number == 2) {
				judge_pattern_two_rightup(p);
			}
			else {
				judge_pattern_two_leftup(p);
			}
		}
		// 장판 움직이기
		switch (pattern_two_number) {
		case 0: {
			for (int i = 0; i < 3; i++) {
				pattern_two_position[i].first -= movesize;
				pattern_two_position[i].second -= movesize;
			}
			for (int i = 0; i < 4; i++) {
				pattern_two_safe_zone[i].first -= movesize;
				pattern_two_safe_zone[i].second -= movesize;
			}

			break;
		}
		case 1: {
			for (int i = 0; i < 3; i++) {
				pattern_two_position[i].first += movesize;
				pattern_two_position[i].second -= movesize;
	
			}
			for (int i = 0; i < 4; i++) {
				pattern_two_safe_zone[i].first += movesize;
				pattern_two_safe_zone[i].second -= movesize;
			}
			break;
		}
		case 2: {
			for (int i = 0; i < 3; i++) {
				pattern_two_position[i].first += movesize;
				pattern_two_position[i].second += movesize;
			}
			for (int i = 0; i < 4; i++) {
				pattern_two_safe_zone[i].first += movesize;
				pattern_two_safe_zone[i].second += movesize;
			}
			break;
		}
		case 3: {
			for (int i = 0; i < 3; i++) {
				pattern_two_position[i].first -= movesize;
				pattern_two_position[i].second += movesize;
			}
			for (int i = 0; i < 4; i++) {
				pattern_two_safe_zone[i].first -= movesize;
				pattern_two_safe_zone[i].second += movesize;
			}

			break;
		}
		}


		for(int i=0; i<GAIA_ROOM; i++) send_gaia_pattern_two_packet(party[i], pattern_two_position, pattern_two_number);
		// 타이머
		pattern_two_count++;
		if (pattern_two_count <= 20) {
			timer_event ev;
			ev.obj_id = dungeon_id;
			ev.start_time = chrono::system_clock::now() + 500ms;
			ev.ev = EVENT_GAIA_PATTERN;
			ev.target_id = 1;
			timer_queue.push(ev);
		}
		else {
			pattern_two_count = 0;
			for (int i = 0; i < GAIA_ROOM; i++) send_gaia_pattern_finish_packet(party[i], 1);
			running_pattern = false;
		}
		break;
	}
	case 4: {
		// 쳐 맞는 판정
		int t_x = pattern_five_position[0].first + 5 * sqrt(26) * boss->get_look_x();
		int t_z = pattern_five_position[0].second + 5 * sqrt(26) * boss->get_look_z();

		float cos_rect = 1 / sqrt(26);
		float sin_rect = 5 / sqrt(26);

		pos rect[4];
		rect[0] = pos(cos_rect * t_x - sin_rect * t_z, sin_rect * t_x + cos_rect * t_z);
		rect[1] = pos(-cos_rect * t_x - sin_rect * t_z, sin_rect * t_x - cos_rect * t_z);
		rect[2] = pos(-cos_rect * t_x + sin_rect * t_z, -sin_rect * t_x - cos_rect * t_z);
		rect[3] = pos(cos_rect * t_x + sin_rect * t_z, -sin_rect * t_x + cos_rect * t_z);


		for (auto& p : party) {

			p->state_lock.lock();
			if (p->get_state() != ST_INDUN) {
				p->state_lock.unlock();
				continue;
			}
			p->state_lock.unlock();

			if (isInsideTriangle(rect[0], rect[1], rect[2], pos(p->get_x(), p->get_z())) 
				|| isInsideTriangle(rect[0], rect[2], rect[3], pos(p->get_x(), p->get_z()))) {
				// 쳐 맞는 판정
				p->set_hp(p->get_hp() - 100);
				if (p->get_hp() < 0) player_death(p);
				for (auto send_pl : party) {
					send_change_hp_packet(send_pl, p);
				}
			}
		}
		// 이동
		pattern_five_position[0].first += boss->get_look_x() * 5;
		pattern_five_position[0].second += boss->get_look_z() * 5;
		for (int i = 0; i < GAIA_ROOM; i++) send_gaia_pattern_five_packet(party[i], &pattern_five_position[0]);
		// 타이머
		pattern_five_count++;
		if (pattern_five_count <= 30) {
			timer_event ev;
			ev.obj_id = dungeon_id;
			ev.start_time = chrono::system_clock::now() + 100ms;
			ev.ev = EVENT_GAIA_PATTERN;
			ev.target_id = 4;
			timer_queue.push(ev);
		}
		else {
			pattern_five_count = 0;
			for (int i = 0; i < GAIA_ROOM; i++) send_gaia_pattern_finish_packet(party[i], 4);
			running_pattern = false;
		}

		break;
	}
	default:
		cout << "잘봇된 패턴 활성화" << endl;
		break;
	}


}
