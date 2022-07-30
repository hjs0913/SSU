#include "TimerManager.h"
#include "Npc.h"
#include "send.h"
#include "LuaFunction.h"
#include "ObjectManager.h"
#include <queue>
#include <random>

default_random_engine dre(std::chrono::system_clock::now().time_since_epoch().count());
uniform_int_distribution<int> rng_move(-1000, 1000);
uniform_int_distribution<int> rng_move_time(1000, 3000);

Npc::Npc(int id)
{
	_id = id;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;
	_move_active = false;

	_x = 300;
	_y = 0;
	_z = 300;

	_look_x = 0.0f;
	_look_y = 0.0f;
	_look_z = 1.0f;

	_right_x = 1.0f;
	_right_y = 0.0f;
	_right_z = 0.0f;
	superposition = false;
	
	_mon_species = static_cast<MONSTER_SPECIES>((_id - NPC_ID_START) / NPC_INTERVAL);
	_target_id = -1;
	burn_on = false;
}

Npc::Npc(int id, const char* name)
{
	sprintf_s(_name, MAX_NAME_SIZE, name);
	_id = id;
	_x = rand() % WORLD_WIDTH;
	_y = rand() % WORLD_HEIGHT;
	_lv = 1;
	_hp = 100;
	_mp = 50;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;
	_move_active = false;

	_look_x = 0.0f;
	_look_y = 0.0f;
	_look_z = 1.0f;

	_right_x = 1.0f;
	_right_y = 0.0f;
	_right_z = 0.0f;

	_target_id = -1;
	superposition = false;
	_mon_species = static_cast<MONSTER_SPECIES>((_id - NPC_ID_START) / NPC_INTERVAL);
	burn_on = false;
}

Npc::~Npc() 
{
}

void Npc::Initialize_Lua(const char* f_lua)
{
	L =  luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L,f_lua) || lua_pcall(L, 0, 0, 0);

	if (error != 0) {
		cout << "초기화 오류" << endl;
	}

	lua_getglobal(L, "Initailize_position");
	error = lua_pcall(L, 0, 4, 0);
	uniform_int_distribution<int> rng_x(lua_tointeger(L, -4), lua_tointeger(L, -3));
	uniform_int_distribution<int> rng_z(lua_tointeger(L, -2), lua_tointeger(L, -1));
	uniform_int_distribution<int> rng_look(-1000, 1000);
	lua_pop(L, 5);

	// 위치 잡기
	_x = rng_x(dre);
	_z = rng_z(dre);

	_look_x = (float)rng_look(dre)/1000.f;
	_look_z = (1.f - fabs(_look_x))*pow(-1, rng_look(dre)%2);

	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, _id);
	lua_pushnumber(L, _x);
	lua_pushnumber(L, _y);
	lua_pushnumber(L, _z);
	error = lua_pcall(L, 4, 10, 0);

	_element = static_cast<ELEMENT>(lua_tointeger(L, -10));
	_lv = lua_tointeger(L, -9);
	set_name(lua_tostring(L, -8));
	_hp = lua_tointeger(L, -7);
	_maxhp = lua_tointeger(L, -7);
	_physical_attack = lua_tonumber(L, -6);
	_magical_attack = lua_tonumber(L, -5);
	_physical_defence = lua_tonumber(L, -4);
	_magical_defence = lua_tonumber(L, -3);
	_basic_attack_factor = lua_tointeger(L, -2);
	_defence_factor = lua_tonumber(L, -1);

	lua_pop(L, 11);// eliminate set_uid from stack after call

	lua_register(L, "API_get_x", API_get_x);
	lua_register(L, "API_get_y", API_get_y);
	lua_register(L, "API_get_z", API_get_z);
}

void Npc::Initialize_Lua_Boss(const char* f_lua, int dungeon_id)
{
	_tribe = BOSS;

	L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L, f_lua) || lua_pcall(L, 0, 0, 0);

	if (error != 0) {
		cout << "초기화 오류" << endl;
	}

	Initialize_Boss(dungeon_id);
}

void Npc::Initialize_Boss(int dungeon_id)
{
	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, dungeon_id);
	int error = lua_pcall(L, 1, 10, 0);

	set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
	set_lv(lua_tointeger(L, -9));

	set_name(lua_tostring(L, -8));

	set_hp(lua_tointeger(L, -7));
	set_maxhp(lua_tointeger(L, -7));

	set_physical_attack(lua_tonumber(L, -6));
	set_magical_attack(lua_tonumber(L, -5));
	set_physical_defence(lua_tonumber(L, -4));
	set_magical_defence(lua_tonumber(L, -3));
	set_basic_attack_factor(lua_tointeger(L, -2));
	set_defence_factor(lua_tonumber(L, -1));

	lua_pop(L, 11);// eliminate set_uid from stack after call

	state_lock.lock();
	set_state(ST_FREE);
	state_lock.unlock();
	set_id(GAIA_ID);

	_x = 310;
	_z = 110;
}

void Npc::set_pos(int x, int z)
{
	_x = x;
	_z = z;
}


void Npc::set_state(STATE s)
{
	_state = s;
}

void Npc::set_name(const char* name)
{
	strncpy_s(_name, name, sizeof(_name));
}

void Npc::set_active(bool act)
{
	_active = act;
}

void Npc::set_move_active(bool mv_act)
{
	_move_active = mv_act;
}

void Npc::set_tribe(TRIBE tribe)
{
	_tribe = tribe;
}

void Npc::set_lv(short lv)
{
	_lv = lv;
}

void Npc::set_hp(int hp)
{
	_hp = hp;
	if (_hp <= 0) _hp = 0;
}

void Npc::set_maxhp(int m_hp)
{
	_maxhp = m_hp;
}

void Npc::set_mp(int mp)
{
	_mp = mp;
}

void Npc::set_maxmp(int m_mp)
{
	_maxmp = m_mp;
}

void Npc::set_physical_attack(float physical_attack)
{
	_physical_attack = physical_attack;
}

void Npc::set_magical_attack(float magical_attack)
{
	_magical_attack = magical_attack;
}

void Npc::set_physical_defence(float physical_defence)
{
	_physical_defence = physical_defence;
}

void Npc::set_magical_defence(float magical_defence)
{
	_magical_defence = magical_defence;
}

void Npc::set_basic_attack_factor(float basic_attack)
{
	_basic_attack_factor = basic_attack;
}

void Npc::set_defence_factor(float defence_factor)
{
	_defence_factor = defence_factor;
}

void Npc::set_look(float look_x, float look_y, float look_z)
{
	_look_x = look_x;
	_look_y = look_y;
	_look_z = look_z;
}

void Npc::set_right(float right_x, float right_y, float right_z)
{
	_right_x = right_x;
	_right_y = right_y;
	_right_z = right_z;
}

void Npc::set_mon_species(int s)
{
	_mon_species = static_cast<MONSTER_SPECIES>(s);
}


// get
char* Npc::get_name()
{
	return _name;
}

STATE Npc::get_state()
{
	return _state;
}

bool Npc::get_active()
{
	return _active;
}

bool Npc::get_move_active()
{
	return _move_active;
}

TRIBE Npc::get_tribe()
{
	return _tribe;
}

short Npc::get_lv()
{
	return _lv;
}

int Npc::get_hp()
{
	return _hp;
}

int Npc::get_maxhp()
{
	return _maxhp;
}

int Npc::get_mp()
{
	return _mp;
}

int Npc::get_maxmp()
{
	return _maxmp;
}
float Npc::get_physical_attack()
{
	return _physical_attack;
}
float Npc::get_magical_attack()
{
	return _magical_attack;
}
float Npc::get_physical_defence()
{
	return _physical_defence;
}
float Npc::get_magical_defence()
{
	return _magical_defence;
}


float Npc::get_basic_attack_factor()
{
	return _basic_attack_factor;
}

float Npc::get_defence_factor()
{
	return _defence_factor;
}

float Npc::get_look_x()
{
	return _look_x;
}

float Npc::get_look_y()
{
	return _look_y;
}

float Npc::get_look_z()
{
	return _look_z;
}

float Npc::get_right_x()
{
	return _right_x;
}

float Npc::get_right_y()
{
	return _right_y;
}

float Npc::get_right_z()
{
	return _right_z;
}

MONSTER_SPECIES Npc::get_mon_spices()
{
	return _mon_species;
}

//origin
float Npc::get_origin_physical_attack()
{
	return _origin_physical_attack;
}
float Npc::get_origin_magical_attack()
{
	return _origin_magical_attack;
}
float Npc::get_origin_physical_defence()
{
	return _origin_physical_defence;
}
float Npc::get_origin_magical_defence()
{
	return _origin_magical_defence;
}
void Npc::set_origin_physical_attack(float physical_attack)
{
	_origin_physical_attack = physical_attack;
}

void Npc::set_origin_magical_attack(float magical_attack)
{
	_origin_magical_attack = magical_attack;
}

void Npc::set_origin_physical_defence(float physical_defence)
{
	_origin_physical_defence = physical_defence;
}

void Npc::set_origin_magical_defence(float magical_defence)
{
	_origin_magical_defence = magical_defence;
}

//skill
void Npc::set_skill_factor(int skill_type, int skill_num)
{

	switch (skill_type)
	{
	case 0:  //물리
		_skill_factors[skill_type][skill_num] = _lv * 0.5 * (skill_num + 1) + 50;
		break;
	case 1:  //마법
		_skill_factors[skill_type][skill_num] = _lv * 0.24 * (skill_num + 1) + 50;
		break;
	case 2:  // 버프 
		break;
	default:
		break;
	}


}

float Npc::get_skill_factor(int skill_type, int skill_num)
{
	return _skill_factors[skill_type][skill_num];
}

void  Npc::set_target_id(int target)
{
	_target_id = target;
}

int  Npc::get_target_id()
{
	return _target_id;
}

void Npc::set_element(ELEMENT element)
{
	_element = element;
}

ELEMENT Npc::get_element()
{
	return _element;
}

void Npc::set_element_cooltime(bool yn)
{
	superposition = yn;
}

bool Npc::get_element_cooltime()
{
	return superposition;
}

bool Npc::check_move_alright(int x, int z, bool monster, const array<Obstacle*, MAX_OBSTACLE>& obstacles)
{
	int size = 0;
	if (monster) size = 15;
	else size = 5;

	for (int i = 0; i < MAX_OBSTACLE; i++) {
		if ((obstacles[i]->get_x() - size <= x && x <= obstacles[i]->get_x() + size) &&
			(obstacles[i]->get_z() - size <= z && z <= obstacles[i]->get_z() + size)) {
			return false;
		}
	}

	for (int i = NPC_ID_START; i < NPC_ID_END; i++) {
		if (i == _id) continue;
		if ((static_ObjectManager::get_objManger()->get_player(i)->get_x() - size <= x &&
			x <= static_ObjectManager::get_objManger()->get_player(i)->get_x() + size) &&
			(static_ObjectManager::get_objManger()->get_player(i)->get_z() - size <= z && 
				z <= static_ObjectManager::get_objManger()->get_player(i)->get_z() + size)) {
			return false;
		}
	}

	return true;
}

int Npc::huristic(int t_x, int t_z, int x, int z)
{
	int s_x = t_x - x;
	int s_z = t_z - z;
	int score = sqrt(pow(s_x, 2) + pow(s_z, 2));
	return score;
}

//move
pos Npc::non_a_star(int t_x, int t_z, int x, int z)
{
	float m_x = t_x - x;
	float m_z = t_z - z;

	float sum = (m_x * m_x) + (m_z * m_z);
	sum = sqrt(sum);

	if (sum == 0) return pos(x, z);

	m_x = m_x / sum;
	m_z = m_z / sum;

	int temp_x = x + m_x * REAL_DISTANCE*2;
	int temp_z = z + m_z * REAL_DISTANCE*2;

	_look_x = m_x;
	_look_z = m_z;

	_right_x = _look_z;
	_right_z = -_look_x;

	return pos(temp_x, temp_z);
}


pos Npc::a_star(int t_x, int t_z, int x, int z, const array<Obstacle*, MAX_OBSTACLE>& obstacles)
{
	vector<pos> mon_load;
	// 쫒아가는 범위는 한 방향으로 60까지이다
	int scoreG[25][25] = { 0 };
	int scoreH[25][25] = { 0 };
	int scoreF[25][25] = { 0 };
	pos prior_point[25][25]{ pos(0,0) };

	typedef pair<int, pos> weight;



	pos now(12, 12);
	scoreG[now.first][now.second] = 0;
	scoreH[now.first][now.second] = huristic(t_x, t_z, x, z);
	scoreF[now.first][now.second] = scoreG[now.first][now.second] + scoreH[now.first][now.second];

	priority_queue < weight, vector<weight>, greater<weight>> open_q;
	priority_queue < weight, vector<weight>, greater<weight>> close_q;
	close_q.push(weight(scoreF[now.first][now.second], now));


	int dirX[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
	int dirZ[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
	int cost[8]{ 10, 10, 10, 10, 14, 14, 14, 14 };
	while (true) {
		for (int i = 0; i < 8; i++) {
			pos p(now.first + dirX[i], now.second + dirZ[i]);

			if ((p.first >= 25 || p.first < 0) || (p.second >= 25 || p.second < 0)) continue;
			// 검색된게 있다면 검색을 해주지 않는다
			if (scoreF[now.first + dirX[i]][now.second + dirZ[i]] != 0) continue;
			// 장애물이랑 부딪히는지 확인
			if (false == check_move_alright(x + (p.first - 12) * REAL_DISTANCE, z + (p.second - 12) * REAL_DISTANCE, true, obstacles)) continue;

			scoreG[now.first + dirX[i]][now.second + dirZ[i]] = scoreG[now.first][now.second] + cost[i];
			scoreH[now.first + dirX[i]][now.second + dirZ[i]] = huristic(t_x, t_z, x + (p.first - 12) * REAL_DISTANCE, z + (p.second - 12) * REAL_DISTANCE);
			scoreF[now.first + dirX[i]][now.second + dirZ[i]] = scoreG[now.first + dirX[i]][now.second + dirZ[i]] +
				scoreH[now.first + dirX[i]][now.second + dirZ[i]];

			prior_point[now.first + dirX[i]][now.second + dirZ[i]] = pos(now.first, now.second);

			weight w(scoreF[now.first + dirX[i]][now.second + dirZ[i]], pos(now.first + dirX[i], now.second + dirZ[i]));
			open_q.push(w);
		}
		if (open_q.size() == 0) {
			while (now.first != 12 || now.second != 12) {
				mon_load.push_back(now);
				now = prior_point[now.first][now.second];
			}
			break;
		}
		weight temp = open_q.top();
		open_q.pop();
		now = temp.second;
		close_q.push(temp);

		// 끝내는 조건
		if (now.first == 0 || now.first == 24 || now.second == 0 || now.second == 24) {
			while (now.first != 12 || now.second != 12) {
				mon_load.push_back(now);
				now = prior_point[now.first][now.second];
			}
			break;
		}

		if (abs((x + (now.first - 12) * REAL_DISTANCE) - t_x) <= 10 && abs((z + (now.second - 12) * REAL_DISTANCE) - t_z) <= 10) {
			while (now.first != 12 || now.second != 12) {
				mon_load.push_back(now);
				now = prior_point[now.first][now.second];
			}
			break;
		}
	}

	if (mon_load.size() != 0) {
		x += (mon_load.back().first - 12) * REAL_DISTANCE;
		z += (mon_load.back().second - 12) * REAL_DISTANCE;
		mon_load.pop_back();
	}

	return pos(x, z);
}

bool Npc::npc_attack_validation(Npc* target)
{
	lua_lock.lock();
	lua_getglobal(L, "attack_range");
	lua_pushnumber(L, target->get_id());
	int error = lua_pcall(L, 1, 1, 0);
	if (error != 0) {
		cout << "LUA ATTACK RANGE ERROR" << endl;
	}
	bool m = false;
	m = lua_toboolean(L, -1);
	lua_pop(L, 1);
	lua_lock.unlock();
	if (m) {
		// 공격처리
		_look_x = target->get_x() - _x;
		_look_z = target->get_z() - _z;

		send_animation_attack(reinterpret_cast<Player*>(target), _id);
		basic_attack_success(target);
		return true;
	}
	else {

		if (_active) {
			// 공격은 실패했지만 계속(그렇지만 1초후) 공격시도
			timer_event ev;
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s;
			ev.ev = EVENT_NPC_ATTACK;
			ev.target_id = _target_id;
			TimerManager::timer_queue.push(ev);
		}
		return false;
	}
}

void Npc::attack_dead_judge(Npc* target)
{
	int target_hp = target->get_hp();
	if (target_hp <= 0) {
		target->state_lock.lock();
		if (target->get_state() != ST_INGAME) {
			target->state_lock.unlock();
			return;
		}
		target->set_state(ST_DEAD);
		target->state_lock.unlock();

		_active = false;
		// 죽은것이 플레이어라면 죽었다는 패킷을 보내준다
		send_dead_packet(reinterpret_cast<Player*>(target), this, target);
		send_notice(reinterpret_cast<Player*>(target), "사망했습니다. 10초 후 부활합니다", 1);

		// 3초후 부활하며 부활과 동시에 위치 좌표를 수정해준다
		timer_event ev;
		ev.obj_id = target->get_id();
		ev.start_time = chrono::system_clock::now() + 10s;
		ev.ev = EVENT_PLAYER_REVIVE;
		ev.target_id = 0;
		TimerManager::timer_queue.push(ev);
	}
	else {
		// 플레이어가 공격을 당한 것이므로 hp정보가 바뀌었으므로 그것을 보내주자
		// send_status_change_packet(reinterpret_cast<Player*>(players[target]));

		// 플레이어의 ViewList에 있는 플레이어들에게 보내주자
		send_change_hp_packet(reinterpret_cast<Player*>(target), target, 0);		// 플레이어 데미지

		// hp가 깎이였으므로 hp자동회복을 해주도록 하자
		if (reinterpret_cast<Player*>(target)->_auto_hp == false) {
			timer_event ev;
			ev.obj_id = target->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_AUTO_PLAYER_HP;
			ev.target_id = 0;
			TimerManager::timer_queue.push(ev);
			reinterpret_cast<Player*>(target)->_auto_hp = true;
		}

		// npc공격이면 타이머 큐에 다시 넣어주자
		timer_event ev;
		ev.obj_id = _id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_NPC_ATTACK;
		ev.target_id = _target_id;
		TimerManager::timer_queue.push(ev);
	}
}

void Npc::attack_element_judge(Npc* target)
{
	if (target->get_element_cooltime() == false) {
		switch (_element)
		{
		case E_WATER:
			if (target->get_element() == E_FULLMETAL || target->get_element() == E_FIRE
				|| target->get_element() == E_EARTH) {
				target->set_magical_attack(target->get_magical_attack() / 10 * 9);
				target->set_element_cooltime(true);
			}

			break;
		case E_FULLMETAL:
			if (target->get_element() == E_ICE || target->get_element() == E_TREE
				|| target->get_element() == E_WIND) {
				_physical_defence += _physical_defence / 10;
				target->set_element_cooltime(true);
			}
			break;
		case E_WIND:
			if (target->get_element() == E_WATER || target->get_element() == E_EARTH
				|| target->get_element() == E_FIRE) {
				reinterpret_cast<Player*>(target)->attack_speed_up = 1;
				target->set_element_cooltime(true);
			}
			break;
		case E_FIRE:
			if (target->get_element() == E_ICE || target->get_element() == E_TREE
				|| target->get_element() == E_FULLMETAL) {
				//10초 공격력 10프로의 화상 피해 
				target->set_element_cooltime(true);
			}
			break;
		case E_TREE:
			if (target->get_element() == E_EARTH || target->get_element() == E_WATER
				|| target->get_element() == E_WIND) {
				target->set_physical_attack(target->get_physical_attack() / 10 * 9);
				target->set_element_cooltime(true);
			}
			break;
		case E_EARTH:
			if (target->get_element() == E_ICE || target->get_element() == E_FULLMETAL
				|| target->get_element() == E_FIRE) {
				_magical_defence += _magical_defence / 10;
				target->set_element_cooltime(true);
			}
			break;
		case E_ICE:
			if (target->get_element() == E_TREE || target->get_element() == E_WATER
				|| target->get_element() == E_WIND) {
				//동결 and  10초동안 공속, 시전속도, 이동속도 10프로감소 
				reinterpret_cast<Player*>(target)->attack_speed_up = -1;
				target->set_element_cooltime(true);
			}
			break;
		default:
			break;
		}
		if (target->get_element_cooltime() == true) {
			timer_event ev;
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
			ev.ev = EVENT_ELEMENT_COOLTIME;;
			ev.target_id = target->get_id();
			TimerManager::timer_queue.push(ev);
		}
	}
}

void Npc::basic_attack_success(Npc* target)
{
	// 현재 물리 공격에 대해서만 생각한다
	float give_damage = _physical_attack * _basic_attack_factor;
	float defence_damage = (target->get_defence_factor() *
		target->get_physical_defence()) / (1 + (target->get_defence_factor() *
			target->get_physical_defence()));
	float damage = give_damage * (1 - defence_damage);
	int target_hp = target->get_hp() - damage;

	if (target_hp <= 0) target_hp = 0;
	target->set_hp(target_hp);

	attack_element_judge(target);

	attack_dead_judge(target);
}

void Npc::phisical_skill_success(Npc* target, float skill_factor)
{
	float give_damage = _physical_attack * skill_factor;
	float defence_damage = (target->get_defence_factor() *
		target->get_physical_defence()) / (1 + (target->get_defence_factor() *
			target->get_physical_defence()));
	float damage = give_damage * (1 - defence_damage);
	int target_hp = target->get_hp() - damage;

	if (target_hp <= 0) target_hp = 0;
	target->set_hp(target_hp);

	attack_element_judge(target);

	attack_dead_judge(target);
}

void Npc::magical_skill_success(Npc* target, float skill_factor)
{
	float give_damage = _magical_attack * skill_factor;
	float defence_damage = (target->get_defence_factor() *
		target->get_magical_defence()) / (1 + (target->get_defence_factor() *
			target->get_magical_defence()));
	float damage = give_damage * (1 - defence_damage);
	int target_hp = target->get_hp() - damage;

	if (target_hp <= 0) target_hp = 0;
	target->set_hp(target_hp);

	attack_element_judge(target);

	attack_dead_judge(target);
}


void Npc::return_npc_position(const array<Obstacle*, MAX_OBSTACLE>& obstacles)
{
	// 여기서는 이동만 시킨다

	if (_active == true) {
		if (_move_active) push_npc_move_event();
		return;
	}

	// 원래 자리로 돌아가자
	lua_lock.lock();
	lua_getglobal(L, "return_my_position");
	int error = lua_pcall(L, 0, 3, 0);
	if (error != 0) {
		lua_lock.unlock();
		cout << "LUA_RETURN_MY_POSITION ERROR" << endl;
		return;
	}

	float my_x = lua_tointeger(L, -3);
	float my_y = lua_tointeger(L, -2);
	float my_z = lua_tointeger(L, -1);
	lua_pop(L, 3);
	lua_lock.unlock();
	
	int now_x = _x;
	int now_y = _y;
	int now_z = _z;
	bool my_pos_fail = true;

	pos mv = a_star(my_x, my_z, now_x, now_z, obstacles);

	if (abs(mv.first - my_x) <= 10 && abs(mv.second - my_z) <= 10) {
		now_x = my_x;
		now_z = my_z;
		my_pos_fail = false;
	}
	else {
		now_x = mv.first;
		now_z = mv.second;
	}

	_look_x = now_x - _x;
	_look_z = now_z - _z;

	_x = now_x;
	_z = now_z;

	//if (my_pos_fail) push_npc_move_event(); // 더 움직여야돼
	push_npc_move_event();
}

void Npc::push_npc_move_event()
{
	timer_event ev;
	ev.obj_id = _id;
	if(_active) ev.start_time = chrono::system_clock::now() + 500ms;
	else ev.start_time = chrono::system_clock::now() + std::chrono::milliseconds(rng_move_time(dre));
	ev.ev = EVENT_NPC_MOVE;
	ev.target_id = _target_id;
	TimerManager::timer_queue.push(ev);
}

void Npc::do_npc_move(Npc* target, const array<Obstacle*, MAX_OBSTACLE>& obstacles)
{
	lua_lock.lock();
	lua_getglobal(L, "event_npc_move");
	lua_pushnumber(L, target->get_id());
	int error = lua_pcall(L, 1, 1, 0);
	if (error != 0) {
		cout << "LUA_NPC_MOVE ERROR" << endl;
	}
	// bool값도 리턴을 해주자 
	// true면 쫒아간다 
	bool m = lua_toboolean(L, -1);
	lua_pop(L, 1);
	lua_lock.unlock();
	if (!m) {
		_active = false;
		_target_id = -1;
		return_npc_position(obstacles);
		return;
	}

	int x = _x;
	int z = _z;

	int t_x = target->get_x();
	int t_z = target->get_z();

	_target_id = target->get_id();

	// 움직일 필요가 없다
	if ((t_x >= x - REAL_DISTANCE*1.5 && t_x <= x + REAL_DISTANCE * 1.5) && (t_z >= z - REAL_DISTANCE * 1.5 && t_z <= z + REAL_DISTANCE * 1.5)) {
		state_lock.lock();
		if (_state != ST_INGAME) {
			state_lock.unlock();
			return;
		}
		state_lock.unlock();

		push_npc_move_event();
		return;
	}

	// A*알고리즘
	pos mv = a_star(t_x, t_z, x, z, obstacles);

	x = mv.first;
	z = mv.second;

	_look_x = x - _x;
	_look_z = z - _z;

	_x = x;
	_z = z;

	state_lock.lock();
	if (_state != ST_INGAME) {
		state_lock.unlock();
		return;
	}
	state_lock.unlock();

	push_npc_move_event();
}

void Npc::npc_roming(const array<Obstacle*, MAX_OBSTACLE>& obstacles)
{
	// 제자리로 돌아가는 것인가? 로밍인가?
	lua_lock.lock();
	lua_getglobal(L, "return_my_position");
	int error = lua_pcall(L, 0, 3, 0);
	if (error != 0) {
		cout << "초기화 오류" << endl;
	}
	int my_x = lua_tonumber(L, -3);
	int my_y = lua_tonumber(L, -2);
	int my_z = lua_tonumber(L, -1);
	lua_pop(L, 3);
	lua_lock.unlock();

	// viewlist따오기
	vl.lock();
	unordered_set<int>my_vl{ viewlist };
	vl.unlock();

	if (_tribe == AGRO) {
		// 범위 지정(가까운 얘를 따라가도록 하자) 
		// -> 애초에 처음 한번이다 2명이 갑자기 들어올 확률이 적다 그냥 바로 따라가주도록 하자
		for (int i : my_vl) {
			if (static_ObjectManager::get_objManger()->get_player(i)->get_tribe() == HUMAN) {
				// 범위 측정
				int h_x = static_ObjectManager::get_objManger()->get_player(i)->get_x();
				int h_z = static_ObjectManager::get_objManger()->get_player(i)->get_z();

				if (sqrt(pow((_x - h_x), 2) + pow((_z - h_z), 2)) <= AGRORANGE) {
					_active = true;
					_target_id = i;

					// 공격 넣어주기
					timer_event ev;
					ev.obj_id = _id;
					ev.start_time = chrono::system_clock::now() + 1s;
					ev.ev = EVENT_NPC_ATTACK;
					ev.target_id = _target_id;
					TimerManager::timer_queue.push(ev);

					// 이동 넣어주기
					push_npc_move_event();
					return;
				}
			}
		}
	}

	if (_x <= my_x - 20 || _x >= my_x + 20 || _z <= my_z - 20 || _z >= my_z + 20) {
		return_npc_position(obstacles);
		return;
	}

	float l_x = rng_move(dre) / 1000.f;
	float l_z = (1.f - fabs(l_x)) * pow(-1, rng_move(dre) % 2);

	int mv_x = _x + l_x * 10;
	int mv_z = _z + l_z * 10;
	
	if (check_move_alright(mv_x, mv_z, true, obstacles)) {
		_look_x = l_x;
		_look_z = l_z;
		_x = mv_x;
		_z = mv_z;
	}

	for (int i : my_vl) {
		if (static_ObjectManager::get_objManger()->get_player(i)->get_tribe() == HUMAN) {
			push_npc_move_event();
			return;
		}
	}
	_move_active = false;

}

void Npc::revive()
{
	// 상태 바꿔주고
	state_lock.lock();
	_state = ST_INGAME;
	state_lock.unlock();

	// NPC의 정보 가져오기
	lua_lock.lock();
	lua_getglobal(L, "monster_revive");
	int error = lua_pcall(L, 0, 4, 0);
	if (error != 0) {
		cout << "초기화 오류" << endl;
	}

	_x = lua_tonumber(L, -4);
	_y = lua_tonumber(L, -3);
	_z = lua_tonumber(L, -2);
	_hp = lua_tointeger(L, -1);
	lua_pop(L, 4);
	lua_lock.unlock();
}
