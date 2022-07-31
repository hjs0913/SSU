#include "ObjectManager.h"
#include "TimerManager.h"
#include "send.h"
#include "Partner.h"
#include <random>
#include <ctime>


Partner::Partner(int d_id) : Player(d_id)
{
	start_game = false;
	target_id = 0;
	join_dungeon_room = false;

	running_pattern = false;
	running_attack =  false;
	pos nearest = { 0,0, };
	float dis = 0.0;
	int nearest_num = 0;
	move_once = false;
	
	party_id = 0;
	skill_check = false;
}

Partner::~Partner()
{
	
}

bool Partner::check_inside(pos a, pos b, pos c, pos n)
{
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

bool Partner::isInsideTriangle(pos a, pos b, pos c, pos n)
{
	if (!check_inside(a, b, c, n)) return false;
	if (!check_inside(b, c, a, n)) return false;
	if (!check_inside(c, a, b, n)) return false;
	return true;
}

void Partner::partner_move(Partner* pa, Gaia* gaia)  
{
	if (running_pattern ) //|| running_attack) // && skill_check
		return;

	switch (pa->get_job()) // AI의 직업을 보고 움직임을 나누자 
	{
	case J_DILLER: {      //전사류는 일단 보스몬스터를 따라가자 
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안하면 붙어
			if (sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) < 10.0f) return;
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			if (static_ObjectManager::get_objManger()->check_move_alright_indun(mv.first, mv.second)) {
				pa->set_x(mv.first);
				pa->set_z(mv.second);
			}
		}
		else if (gaia->running_pattern == true) {  //보스가 패턴을 쓸 때,  패턴의 번호를 받아서 피할 수 있도록 하자 // 0.장판4개 1.날아오기3개  4.참격1개 
			switch (gaia->pattern_num)
			{
			case 0:
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_one_position[i].first;
					int z = gaia->pattern_one_position[i].second;

					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}

				}
				move = pa->non_a_star(gaia->pattern_one_position[nearest_num].first + 30, gaia->pattern_one_position[nearest_num].second, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			case 1:    //  안전지대 2개 중 가까운데로 가자 
				dis = 0;
					for (int i = 0; i < 4; i++) {
						int x = gaia->pattern_two_safe_zone[i].first;
						int z = gaia->pattern_two_safe_zone[i].second;
						if (i == 0)
							dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						else {
							if (dis > sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
								dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
								nearest_num = i;
							}
						}
					}
					move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first, gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());
					if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
						pa->set_x(move.first);
						pa->set_z(move.second);
					}
				
				break;
			case 4:
				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 30, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 30, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_TANKER: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안하면 붙어
			if (sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) < 10.0f) return;
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			if (static_ObjectManager::get_objManger()->check_move_alright_indun(mv.first, mv.second)) {
				pa->set_x(mv.first);
				pa->set_z(mv.second);
			}
		}
		else if (gaia->running_pattern == true) {  //보스가 패턴을 쓸 때,  패턴의 번호를 받아서 피할 수 있도록 하자 // 0.장판4개 1.날아오기3개  4.참격1개 
			switch (gaia->pattern_num)
			{
			case 0:
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_one_position[i].first;
					int z = gaia->pattern_one_position[i].second;

					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}

				}
				move = pa->non_a_star(gaia->pattern_one_position[nearest_num].first + 30, gaia->pattern_one_position[nearest_num].second, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			case 1:    //  안전지대 2개 중 가까운데로 가자 
				dis = 0;
				for (int i = 0; i < 2; i++) {
					int x = gaia->pattern_two_safe_zone[i].first;
					int z = gaia->pattern_two_safe_zone[i].second;
					if (i == 0)
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
					else {
						if (dis > sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
							dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
							nearest_num = i;
						}
					}

				}
				move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first, gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());

				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}

				break;
			case 4:
				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 30, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 30, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_MAGICIAN: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안해도 그래도 일정기리는 유지해   // 일단 보스가 바라보는 반대방향?
			//pa->set_look(-gaia->boss->get_x(), gaia->boss->get_y(), -gaia->boss->get_z());
			if (sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) > 50.0f &&
				sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) < 71.0f) {
				pa->set_look((-1) * gaia->boss->get_look_x(), pa->get_look_y(), (-1) * gaia->boss->get_look_z());
				return;
			}
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() + gaia->boss->get_look_x() * 65, gaia->boss->get_z() + gaia->boss->get_look_z() * 65, pa->get_x(), pa->get_z());
			if (static_ObjectManager::get_objManger()->check_move_alright_indun(mv.first, mv.second)) {
				pa->set_x(mv.first);
				pa->set_z(mv.second);
			}
		}
		else if (gaia->running_pattern == true) {  //보스가 패턴을 쓸 때,  패턴의 번호를 받아서 피할 수 있도록 하자 // 0.장판4개 1.날아오기3개  4.참격1개 
			switch (gaia->pattern_num)
			{
			case 0:
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_one_position[i].first;
					int z = gaia->pattern_one_position[i].second;

					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}

				}
				move = pa->non_a_star(gaia->pattern_one_position[nearest_num].first + 30, gaia->pattern_one_position[nearest_num].second, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			case 1:    //  안전지대 2개 중 가까운데로 가자 
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_two_safe_zone[i].first;
					int z = gaia->pattern_two_safe_zone[i].second;
					if (i == 0)
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
					else {
						if (dis > sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
							dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
							nearest_num = i;
						}
					}

				}
				move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first, gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());

				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}

				break;
			case 4:
				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 30, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 30, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_SUPPORTER: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안해도 그래도 일정기리는 유지해 
		//	if (sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) < 50.0f) return;
			if (sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) > 50.0f &&
				sqrt(pow((gaia->get_x() - pa->get_x()), 2) + pow((gaia->get_z() - pa->get_z()), 2)) < 71.0f)
				return;
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() - gaia->boss->get_look_x() * 65, gaia->boss->get_z() + gaia->boss->get_look_z() * 65, pa->get_x(), pa->get_z());
			if (static_ObjectManager::get_objManger()->check_move_alright_indun(mv.first, mv.second)) {
				pa->set_x(mv.first);
				pa->set_z(mv.second);
			}
		}
		else if (gaia->running_pattern == true) {  //보스가 패턴을 쓸 때,  패턴의 번호를 받아서 피할 수 있도록 하자 // 0.장판4개 1.날아오기3개  4.참격1개 
			switch (gaia->pattern_num)
			{
			case 0:
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_one_position[i].first;
					int z = gaia->pattern_one_position[i].second;

					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}

				}
				move = pa->non_a_star(gaia->pattern_one_position[nearest_num].first + 30, gaia->pattern_one_position[nearest_num].second, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			case 1:    //  안전지대 2개 중 가까운데로 가자 
				dis = 0;
				for (int i = 0; i < 4; i++) {
					int x = gaia->pattern_two_safe_zone[i].first;
					int z = gaia->pattern_two_safe_zone[i].second;
					if (i == 0)
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
					else {
						if (dis > sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
							dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
							nearest_num = i;
						}
					}

				}
				move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first, gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());

				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}

				break;
			case 4:

				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 30, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 30, pa->get_x(), pa->get_z());
				if (static_ObjectManager::get_objManger()->check_move_alright_indun(move.first, move.second)) {
					pa->set_x(move.first);
					pa->set_z(move.second);
				}
				break;
			default:
				break;
			}
		}
		break;
	}
	default:
		break;
	}
}

void Partner::partner_attack(Partner* pa, Gaia* gaia) //스킬을 쿨타임 돌때마다 계속 쓰도록 하자     //  서포타가 오류인가?
{

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 11); //99);
	timer_event ev;
	if (gaia->running_pattern == true || pa->get_mp() < pa->get_lv() * 10 || (pa->get_mp() < pa->get_lv() * 10 && pa->get_hp() < pa->get_lv() * 10)  ) {
		ev.obj_id = _id;
		ev.start_time = chrono::system_clock::now() + 5s;
		ev.ev = EVENT_PARTNER_SKILL;
		ev.target_id = _id;
		TimerManager::timer_queue.push(ev);
		running_pattern = false;
		return;
	}
	int p =  pattern(gen) % 3;

	Player** party_player = gaia->get_party_palyer();

	switch (pa->get_job()) // AI의 직업을 보고 움직임을 나누자 
	{
	case J_DILLER: {
		switch (p)
		{
		case 0: {

			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);
			if ((gaia->boss->get_x() >= pa->get_x() - 30 && gaia->boss->get_x() <= pa->get_x() + 30) && (gaia->boss->get_z() >= pa->get_z() - 30 && gaia->boss->get_z() <= pa->get_z() + 30)) {
				skill_check = true;
				pa->set_skill_factor(0, 0);

				float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_physical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_physical_defence()));
				float damage = give_damage * (1 - defence_damage);

				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);

					for (int i = 0; i < GAIA_ROOM; ++i) {
						send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss, 0);
					}
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}

		
			}

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 2s + 600ms;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			running_pattern = true;
			break;
		}
		case 1: {


			pos a = { pa->get_x(), pa->get_z() };    //플레이어 기준 전방 삼각형 범위 
			pos b = { pa->get_x() - pa->get_right_x() * 30 + pa->get_look_x() * 70,
				pa->get_z() - pa->get_right_z() * 30 + pa->get_look_z() * 70 };  // 왼쪽 위
			pos c = { pa->get_x() + pa->get_right_x() * 30 + pa->get_look_x() * 70,
				pa->get_z() + pa->get_right_z() * 30 + pa->get_look_z() * 70 };  // 오른쪽 위

			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

			pos n = { gaia->boss->get_x(),gaia->boss->get_z() };


			if (isInsideTriangle(a, b, c, n)) {
				skill_check = true;
				pa->set_skill_factor(1, 0);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);
					for (int i = 0; i < GAIA_ROOM; ++i) {
						send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss, 0);
					}
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}
	
			}
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s + 800ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			break;
		}
		case 2: {

			skill_check = true;
			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

			pa->set_physical_attack(0.6 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //일단 두배 
			pa->set_magical_attack(0.2 * pa->get_lv() * pa->get_lv() + 5 * pa->get_lv());
			//send_status_change_packet(pl);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s + 200ms;;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		default:
			break;
		}
		
		if (skill_check) {
			for (int i = 0; i < GAIA_ROOM; i++) {
				if (party_player[i]->get_tribe() == HUMAN)
					send_animation_skill(party_player[i], pa->get_id(), p);
			}
			skill_check = false;
		}
		
		break;
	}
	case J_TANKER: {
		switch (p)
		{
		case 0: {   //밀어내기 공격 

			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

			if ((gaia->boss->get_x() >= pa->get_x() - 20 && gaia->boss->get_x() <= pa->get_x() + 20) && (gaia->boss->get_z() >= pa->get_z() - 20 && gaia->boss->get_z() <= pa->get_z() + 20)) {
				skill_check = true;
				pa->set_skill_factor(0, 0);
				float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_physical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_physical_defence()));
				float damage = give_damage * (1 - defence_damage);
		
				int m_x = 2037;
				int m_z = 2112;
				float r = 515.f;

	
				if (sqrt(pow((gaia->boss->get_x() + pa->get_look_x() * 100 - m_x), 2) + pow((gaia->boss->get_z() + pa->get_look_z() * 100 - m_z), 2)) < r)
					gaia->boss->set_pos(gaia->boss->get_x() + pa->get_look_x() * 100, gaia->boss->get_z() + pa->get_look_z() * 100);
			

				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);
					for (int i = 0; i < GAIA_ROOM; ++i) {
						send_move_packet(gaia->get_party_palyer()[i], gaia->boss, 1);
						send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss, 0);
					}
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}
			}
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);


			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s + 600ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 1: {  //어그로 끌기 

			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);
	
			if ((gaia->boss->get_x() >= pa->get_x() - 40 && gaia->boss->get_x() <= pa->get_x() + 40) && (gaia->boss->get_z() >= pa->get_z() - 40 && gaia->boss->get_z() <= pa->get_z() + 40)) {
				skill_check = true;
				pa->set_skill_factor(1, 0);

				for(int i = 0; i < GAIA_ROOM; ++i) {
					if (gaia->get_party_palyer()[i]->get_id() == pa->get_id())
						gaia->target_id = i;
				}

				//send_status_change_packet(pl);
			} 
		
			ev.obj_id = _id;      // 해제는 나중에 다시 
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 2s;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 2: {  //자기 방어력 증가 

			skill_check = true;
			pa->set_mp(pa->get_mp() - 1000);
			pa->set_physical_defence(0.54 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //일단 두배 
			pa->set_magical_defence(0.4 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv());
			//send_status_change_packet(pl);
			
		            // 해제는 나중에 다시 
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now()  + 300ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		default:
			break;
		}
		if (skill_check) {
			for (int i = 0; i < GAIA_ROOM; i++) {
				if (party_player[i]->get_tribe() == HUMAN)
					send_animation_skill(party_player[i], pa->get_id(), p);
			}
			skill_check = false;
		}
		break;
	}
	case J_MAGICIAN: {
		switch (p)
		{
		case 0: {  //내 피 줄이고 스킬 사용해 몬스터 hp를 깎아 내 mp를 채움  

			pa->set_hp(pa->get_hp() - pa->get_lv() * 10);

			if ((gaia->boss->get_x() >= pa->get_x() - 30 && gaia->boss->get_x() <= pa->get_x() + 30) && (gaia->boss->get_z() >= pa->get_z() - 30 && gaia->boss->get_z() <= pa->get_z() + 30)) {
				skill_check = true;
				pa->set_mp(pa->get_mp() + gaia->boss->get_hp() / 10);
				if (pa->get_mp() > pa->get_maxmp())
					pa->set_mp(pa->get_maxmp());

				pa->set_skill_factor(1, 0);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);

				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);
					for (int i = 0; i < GAIA_ROOM; ++i) {
						send_move_packet(gaia->get_party_palyer()[i], gaia->boss, 1);
						send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss, 0);
						send_change_hp_packet(gaia->get_party_palyer()[i], pa, 0);
					}
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}
			}
		
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 3s + 300ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 1: {  // 메테오, 에너지볼? 

			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

		//	for (int i = 0; i < GAIA_ROOM; ++i)    //이건 그리라고 보내주는거다 // 근데 partner 전용으로 함수 만들어서 처리하자 ****
			//	send_play_shoot_packet(gaia->get_party_palyer()[i]);

			pos a = { pa->get_x() + pa->get_right_x() * -30, pa->get_z() + pa->get_right_z() * -30 };
			pos b = { pa->get_x() + pa->get_right_x() * 30, pa->get_z() + pa->get_right_z() * 30 };
			pos c = { (pa->get_x() + pa->get_right_x() * -30) + pa->get_look_x() * 140,
		   (pa->get_z() + pa->get_right_z() * -30) + pa->get_look_z() * 140, };

			pos d = { pa->get_x() + pa->get_right_x() * 30, pa->get_z() + pa->get_right_z() * 30 };
			pos e = { (pa->get_x() + pa->get_right_x() * 30) + pa->get_look_x() * 140,
				 (pa->get_z() + pa->get_right_z() * 30) + pa->get_look_x() * 100 };
			pos f = { (pa->get_x() + pa->get_right_x() * -30) + pa->get_look_x() * 140,
		   (pa->get_z() + pa->get_right_z() * -30) + pa->get_look_z() * 140, };

			pos n = {gaia->boss->get_x(), gaia->boss->get_z()};

			if (isInsideTriangle(a, b, c, n) || isInsideTriangle(d, e, f, n)) {
				skill_check = true;
				pa->set_skill_factor(1, 1);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 1);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);

					//for (int i = 0; i < GAIA_ROOM; ++i)
					//	send_play_effect_packet(gaia->get_party_palyer()[i], gaia->boss); // 이펙트 터트릴 위치 
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}
			}
		
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);


			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 2s + 700ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 2: {
			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

			pos a1 = { pa->get_x() + pa->get_right_x() * -10, pa->get_z() + pa->get_right_z() * -10 };
			pos b1 = { pa->get_x() + pa->get_right_x() * 10, pa->get_z() + pa->get_right_z() * 10 };
			pos c1 = { (pa->get_x() + pa->get_right_x() * -10) + pa->get_look_x() * 140,
		   (pa->get_z() + pa->get_right_z() * -10) + pa->get_look_z() * 140, };


			pos d1 = { pa->get_x() + pa->get_right_x() * 10, pa->get_z() + pa->get_right_z() * 10 };
			pos e1 = { (pa->get_x() + pa->get_right_x() * 10) + pa->get_look_x() * 140
				, (pa->get_z() + pa->get_right_z() * 10) + pa->get_look_x() * 140 };
			pos f1 = { (pa->get_x() + pa->get_right_x() * -10) + pa->get_look_x() * 140,
		   (pa->get_z() + pa->get_right_z() * -10) + pa->get_look_z() * 140, };

			pos n = { gaia->boss->get_x(), gaia->boss->get_z() };

			if (isInsideTriangle(a1, b1, c1, n) || isInsideTriangle(d1, e1, f1, n)) {
				skill_check = true;
				pa->set_skill_factor(1, 2);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 2);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				if (gaia->boss->get_hp() > 0) {
					gaia->boss->set_hp(gaia->boss->get_hp() - damage);

				//	for (int i = 0; i < GAIA_ROOM; ++i)
					//	send_play_effect_packet(gaia->get_party_palyer()[i], gaia->boss); // 이펙트 터트릴 위치 
					if (gaia->boss->get_hp() <= 0) {
						gaia->boss->set_hp(0);
						gaia->game_victory();
					}
				}
			}
				ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);


			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 2s + 300ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		default:
			break;
		}
		if (skill_check) {
			for (int i = 0; i < GAIA_ROOM; i++) {
				if (party_player[i]->get_tribe() == HUMAN)
					send_animation_skill(party_player[i], pa->get_id(), p);
			}
			skill_check = false;
		}
		break;
	}
	case J_SUPPORTER: {
		switch (p)
		{
		case 0: {  //hp 회복 

			if (gaia->get_party_palyer()[0]->get_maxhp() == gaia->get_party_palyer()[0]->get_hp() &&
				gaia->get_party_palyer()[1]->get_maxhp() == gaia->get_party_palyer()[1]->get_hp() &&
				gaia->get_party_palyer()[2]->get_maxhp() == gaia->get_party_palyer()[2]->get_hp()) {
				ev.obj_id = _id;
				ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				TimerManager::timer_queue.push(ev);
				running_pattern = true;
				return;
			}
				

			int tmp_hp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // 낮은 체력 플레이어 찾기 
				if (gaia->get_party_palyer()[target_player]->get_state() == ST_DEAD)
					continue;

					if (i == 0) {
						target_player = i;
						tmp_hp = gaia->get_party_palyer()[i]->get_hp();

					}
					else {
						if (tmp_hp > gaia->get_party_palyer()[i]->get_hp()) {
							target_player = i;
							tmp_hp = gaia->get_party_palyer()[i]->get_hp();
						}
					}
				
			}
			skill_check = true;
			if (gaia->get_party_palyer()[target_player]->get_state() != ST_DEAD) {
				pa->set_mp(pa->get_mp() - pa->get_lv() * 10);
				send_buff_ui_packet(gaia->get_party_palyer()[target_player], 2); //ui


				if (gaia->get_party_palyer()[target_player]->get_hp() + gaia->get_party_palyer()[target_player]->get_maxhp() / 10 >= gaia->get_party_palyer()[target_player]->get_maxhp())
					gaia->get_party_palyer()[target_player]->set_hp(gaia->get_party_palyer()[target_player]->get_maxhp());
				else
					gaia->get_party_palyer()[target_player]->set_hp(gaia->get_party_palyer()[target_player]->get_hp() + gaia->get_party_palyer()[target_player]->get_maxhp() / 10);

				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_change_hp_packet(gaia->get_party_palyer()[i], gaia->get_party_palyer()[target_player], 0);
				}
			}
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);


			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 2s;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 1: { //mp 회복   //여기이상 

			if (gaia->get_party_palyer()[0]->get_maxmp() == gaia->get_party_palyer()[0]->get_mp() &&
				gaia->get_party_palyer()[1]->get_maxmp() == gaia->get_party_palyer()[1]->get_mp() &&
				gaia->get_party_palyer()[2]->get_maxmp() == gaia->get_party_palyer()[2]->get_mp()) {
				ev.obj_id = _id;
				ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				TimerManager::timer_queue.push(ev);
				running_pattern = true;
				return;
			}
			int tmp_mp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // 낮은 마나 플레이어 찾기 
				if (gaia->get_party_palyer()[i]->get_mp() > 0) {
					if (gaia->get_party_palyer()[target_player]->get_state() == ST_DEAD)
						continue;
					if (i == 0) {
						target_player = i;
						tmp_mp = gaia->get_party_palyer()[i]->get_mp();
					}
					else {
						if (tmp_mp > gaia->get_party_palyer()[i]->get_mp()) {
							target_player = i;
							tmp_mp = gaia->get_party_palyer()[i]->get_mp();
						}
					}
				}
			}
			skill_check = true;
			if (gaia->get_party_palyer()[target_player]->get_state() != ST_DEAD) {
				pa->set_mp(pa->get_mp() - pa->get_lv() * 10);
				send_buff_ui_packet(gaia->get_party_palyer()[target_player], 0); //ui
				if (gaia->get_party_palyer()[target_player]->get_mp() + gaia->get_party_palyer()[target_player]->get_maxmp() / 10 >= gaia->get_party_palyer()[target_player]->get_maxmp())
					gaia->get_party_palyer()[target_player]->set_mp(gaia->get_party_palyer()[target_player]->get_maxmp());
				else
					gaia->get_party_palyer()[target_player]->set_mp(gaia->get_party_palyer()[target_player]->get_mp() + gaia->get_party_palyer()[target_player]->get_maxmp() / 10);

				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_change_mp_packet(gaia->get_party_palyer()[i], gaia->get_party_palyer()[target_player]);
				}
			}
			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s + 800ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		case 2: { // 공속 올리기 

			skill_check = true;

			for (int i = 0; i < GAIA_ROOM; ++i) {
				if (gaia->get_party_palyer()[i]->get_state() != ST_DEAD) {
					gaia->get_party_palyer()[i]->attack_speed_up = 1;
					send_buff_ui_packet(gaia->get_party_palyer()[i], 4);
				}
			}
			pa->set_mp(pa->get_mp() - pa->get_lv() * 10);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;   // 파트너 (버프) 스킬 이벤트를 따로 만들지 생각해보자 
			ev.target_id = _id;  // 일단 이걸로 구분 
			TimerManager::timer_queue.push(ev);

			ev.obj_id = _id;
			ev.start_time = chrono::system_clock::now() + 1s + 800ms;
			ev.ev = EVENT_PARTNER_SKILL_STOP;
			ev.target_id = _id;
			TimerManager::timer_queue.push(ev);
			running_pattern = true;
			break;
		}
		default:
			break;
		}
		if (skill_check) {
			for (int i = 0; i < GAIA_ROOM; i++) {
				if (party_player[i]->get_tribe() == HUMAN)
					send_animation_skill(party_player[i], pa->get_id(), p);
			}
			skill_check = false;
		}
		break;
	}
	default:
		break;
	}
	
}

void Partner::attack_success(Partner* pa, Gaia* gaia, float atk_factor)
{
	float give_damage = pa->get_physical_attack() * atk_factor;
	float defence_damage = (gaia->boss->get_defence_factor() *
		gaia->boss->get_physical_defence()) / (1 + (gaia->boss->get_defence_factor() *
			gaia->boss->get_physical_defence()));
	float damage = give_damage * (1 - defence_damage);
	int target_hp = gaia->boss->get_hp() - damage;

	gaia->boss->set_hp(target_hp);
	for (int i = 0; i < GAIA_ROOM; ++i) {
		send_animation_attack(gaia->get_party_palyer()[i], pa->get_id());
		send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss, 0);
	}
	//hp가 0이되는건 처리 안해놈 -> 하고있음
	if (gaia->boss->get_hp() <= 0) {
		gaia->boss->set_hp(0);
		gaia->game_victory();
	}
}

void Partner::partner_normal_attack(Partner* pa, Gaia* gaia)
{
	if (running_pattern )
		return;

	float partner_x, partner_z;
	float gaia_x, gaia_z;
	partner_x = pa->get_x(); partner_z = pa->get_z();
	gaia_x = gaia->boss->get_x(); gaia_z = gaia->boss->get_z();

	if (gaia_z <= partner_z + 10) {
		if (partner_z - 10 <= gaia_z) {
			if (gaia_x <= partner_x + 10) {
				if (partner_x - 10 <= gaia_x) {
					attack_success(pa, gaia, pa->get_basic_attack_factor());
				}
			}
		}
	}
}

int Partner::get_party_id()
{
	return party_id;
}
void Partner::set_party_id(int id)
{
	party_id = id;
}