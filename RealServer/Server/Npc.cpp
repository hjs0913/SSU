#include "Npc.h"
#include <queue>

Npc::Npc(int id)
{
	_id = id;
	_tribe = MONSTER;
	_state = ST_INGAME;
	_active = false;

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
	// _x = rand() % WORLD_WIDTH;
	// _y = rand() % WORLD_HEIGHT;
	_target_id = -1;
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

	_look_x = 0.0f;
	_look_y = 0.0f;
	_look_z = 1.0f;

	_right_x = 1.0f;
	_right_y = 0.0f;
	_right_z = 0.0f;

	_target_id = -1;
	superposition = false;
}

Npc::~Npc() 
{
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

bool Npc::check_move_alright(int x, int z, bool monster, array<Obstacle, MAX_OBSTACLE> obs)
{
	int size = 0;
	if (monster) size = 15;
	else size = 5;

	for (auto& ob : obs) {
		if ((ob.get_x() - size <= x && x <= ob.get_x() + size) && (ob.get_z() - size <= z && z <= ob.get_z() + size)) {
			cout << "충돌했다" << endl;
			return false;
		}
	}

	return true;
}

int Npc::huristic(int t_x, int t_z, int x, int z)
{
	int s_x = abs(t_x - x);
	int s_z = abs(t_z - z);
	int score = sqrt(pow(s_x, 2) + pow(s_z, 2));
	//cout << "huristic : " << score << endl;
	return score;
}

//move
pos Npc::non_a_star(int t_x, int t_z, int x, int z)
{
	float m_x = t_x - x;
	float m_z = t_z - z;

	float sum = (m_x * m_x) + (m_z * m_z);
	sum = sqrt(sum);

	m_x = m_x / sum;
	m_z = m_z / sum;

	x += m_x * REAL_DISTANCE*2;
	z += m_z * REAL_DISTANCE*2;

	_look_x = m_x;
	_look_z = m_z;

	_right_x = _look_z;
	_right_z = -_look_x;

	return pos(x, z);
}


pos Npc::a_star(int t_x, int t_z, int x, int z,array<Obstacle, MAX_OBSTACLE> obs)
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
			if (false == check_move_alright(x + (p.first - 12) * REAL_DISTANCE, z + (p.second - 12) * REAL_DISTANCE, true, obs)) continue;

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
		if (abs((x + (now.first - 12) * REAL_DISTANCE) - t_x) <= 10 && abs((z + (now.second - 12) * REAL_DISTANCE) - t_z) <= 10) {
			while (now.first != 12 || now.second != 12) {
				mon_load.push_back(now);
				now = prior_point[now.first][now.second];
			}
			break;
		}
	}

	x += (mon_load.back().first - 12) * REAL_DISTANCE;
	z += (mon_load.back().second - 12) * REAL_DISTANCE;
	mon_load.pop_back();

	return pos(x, z);
}
