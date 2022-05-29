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

	pos nearest = { 0,0, };
	float dis = 0.0;
	int nearest_num = 0;
	move_once = false;
	
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
	if (running_pattern) return;

	switch (pa->get_job()) // AI의 직업을 보고 움직임을 나누자 
	{
	case J_DILLER: {      //전사류는 일단 보스몬스터를 따라가자 
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안하면 붙어
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
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
				pa->set_x(move.first);
				pa->set_z(move.second);
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
					pa->set_x(move.first);
					pa->set_z(move.second);
				
				break;
			case 4:
				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 10, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 10, pa->get_x(), pa->get_z());
				pa->set_x(move.first);
				pa->set_z(move.second);
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_TANKER: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안하면 붙어
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
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
				pa->set_x(move.first);
				pa->set_z(move.second);
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

				pa->set_x(move.first);
				pa->set_z(move.second);

				break;
			case 4:
				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 10, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 10, pa->get_x(), pa->get_z());
				pa->set_x(move.first);
				pa->set_z(move.second);
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_MAGICIAN: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안해도 그래도 일정기리는 유지해   // 일단 보스가 바라보는 반대방향?
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() - gaia->boss->get_look_x() * 100, gaia->boss->get_z() + gaia->boss->get_look_z() * 100, pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
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
				pa->set_x(move.first);
				pa->set_z(move.second);
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

				pa->set_x(move.first);
				pa->set_z(move.second);

				break;
			case 4:

				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 10, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 10, pa->get_x(), pa->get_z());
				pa->set_x(move.first);
				pa->set_z(move.second);
				break;
			default:
				break;
			}
		}
		break;
	}
	case J_SUPPORTER: {
		if (gaia->running_pattern == false) {   //보스 패턴을 실행 안해도 그래도 일정기리는 유지해 
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() - gaia->boss->get_look_x() * 100, gaia->boss->get_z() + gaia->boss->get_look_z() * 100, pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
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
				pa->set_x(move.first);
				pa->set_z(move.second);
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

				pa->set_x(move.first);
				pa->set_z(move.second);

				break;
			case 4:

				move = pa->non_a_star(gaia->boss->get_look_x() + gaia->boss->get_right_x() * 10, gaia->boss->get_look_z() + gaia->boss->get_right_z() * 10, pa->get_x(), pa->get_z());
				pa->set_x(move.first);
				pa->set_z(move.second);
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
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen) % 3;

	//일단 직업에 따라서 다시 분류 합시다.
	//그리고 ai는 피킹이 필요없게 하자 // **직업이랑 hp, mp확인후 제일 필요한 사람에게 버프 주고 버프 ui패킷도 보내자 
	switch (pa->get_job()) // AI의 직업을 보고 움직임을 나누자 
	{
	case J_DILLER: {
		switch (p)
		{
		case 0: {
			running_pattern = true;
			pa->set_mp(pa->get_mp() - 1000);
			if ((gaia->boss->get_x() >= pa->get_x() - 10 && gaia->boss->get_x() <= pa->get_x() + 10) && (gaia->boss->get_z() >= pa->get_z() - 10 && gaia->boss->get_z() <= pa->get_z() + 10)) {
				pa->set_skill_factor(0, 0);

				float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_physical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_physical_defence()));
				float damage = give_damage * (1 - defence_damage);
				gaia->boss->set_hp(gaia->boss->get_hp() - damage);

				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
				}
			}

			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 3s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: {
			running_pattern = true;
			pos a = { pa->get_x(), pa->get_z() };    //플레이어 기준 전방 삼각형 범위 
			pos b = { pa->get_x() - pa->get_right_x() * 40 + pa->get_look_x() * 100,
				pa->get_z() - pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // 왼쪽 위
			pos c = { pa->get_x() + pa->get_right_x() * 40 + pa->get_look_x() * 100,
				pa->get_z() + pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // 오른쪽 위

			pa->set_mp(pa->get_mp() - 1000);

			pos n = { gaia->boss->get_x(),gaia->boss->get_z() };


			if (isInsideTriangle(a, b, c, n)) {
				pa->set_skill_factor(1, 0);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				gaia->boss->set_hp(gaia->boss->get_hp() - damage);
				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
				}
			}
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: {
			running_pattern = true;
			pa->set_mp(pa->get_mp() - 1000);

			pa->set_physical_attack(0.6 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //일단 두배 
			pa->set_magical_attack(0.2 * pa->get_lv() * pa->get_lv() + 5 * pa->get_lv());
			//send_status_change_packet(pl);

			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = pa->get_id();;
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		default:
			break;
		}
		break;
	}
	case J_TANKER: {
		switch (p)
		{
		case 0: {   //밀어내기 공격 
			running_pattern = true;
		
			pa->set_mp(pa->get_mp() - 1000);

			if ((gaia->boss->get_x() >= pa->get_x() - 15 && gaia->boss->get_x() <= pa->get_x() + 15) && (gaia->boss->get_z() >= pa->get_z() - 15 && gaia->boss->get_z() <= pa->get_z() + 15)) {
	
				pa->set_skill_factor(0, 0);
				float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_physical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_physical_defence()));
				float damage = give_damage * (1 - defence_damage);
		
				gaia->boss->set_pos(gaia->boss->get_x() + pa->get_look_x() * 40, gaia->boss->get_z() + pa->get_look_z() * 40);
				gaia->boss->set_hp(gaia->boss->get_hp() - damage);
				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_move_packet(gaia->get_party_palyer()[i], gaia->boss, 1);
					send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
				}
			}
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: {  //어그로 끌기 
			running_pattern = true;
	
			pa->set_mp(pa->get_mp() - 1000);
	
			if ((gaia->boss->get_x() >= pa->get_x() - 40 && gaia->boss->get_x() <= pa->get_x() + 40) && (gaia->boss->get_z() >= pa->get_z() - 40 && gaia->boss->get_z() <= pa->get_z() + 40)) {
				pa->set_skill_factor(1, 0);
				gaia->target_id = pa->get_indun_id();
				//send_status_change_packet(pl);
			} 
		
			ev.obj_id = gaia->get_dungeon_id();       // 해제는 나중에 다시 
			ev.start_time = chrono::system_clock::now() + 7s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = pa->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: {  //자기 방어력 증가 
			running_pattern = true;

			pa->set_mp(pa->get_mp() - 1000);
			pa->set_physical_defence(0.54 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //일단 두배 
			pa->set_magical_defence(0.4 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv());
			//send_status_change_packet(pl);
			
		            // 해제는 나중에 다시 
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = 2;
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		default:
			break;
		}
		break;
	}
	case J_MAGICIAN: {
		switch (p)
		{
		case 0: {  //내 피 줄이고 스킬 사용해 몬스터 hp를 깎아 내 mp를 채움  
			running_pattern = true;
		
			pa->set_hp(pa->get_hp() - 300);

			if ((gaia->boss->get_x() >= pa->get_x() - 30 && gaia->boss->get_x() <= pa->get_x() + 30) && (gaia->boss->get_z() >= pa->get_z() - 30 && gaia->boss->get_z() <= pa->get_z() + 30)) {
				
				pa->set_mp(pa->get_mp() + gaia->boss->get_hp() / 10);
				if (pa->get_mp() > pa->get_maxmp())
					pa->set_mp(pa->get_maxmp());

				pa->set_skill_factor(1, 0);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				gaia->boss->set_hp(gaia->boss->get_hp() - damage);

			

				for (int i = 0; i < GAIA_ROOM; ++i) {
					send_move_packet(gaia->get_party_palyer()[i], gaia->boss, 1);
					send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
					send_change_hp_packet(gaia->get_party_palyer()[i], pa);
				}
			}
		
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: {  // 메테오, 에너지볼? 
			running_pattern = true;

			pa->set_mp(pa->get_mp() - 1500);

			for (int i = 0; i < GAIA_ROOM; ++i)    //이건 그리라고 보내주는거다 // 근데 partner 전용으로 함수 만들어서 처리하자 ****
				send_play_shoot_packet(gaia->get_party_palyer()[i]);

			pos a = { pa->get_x() + pa->get_right_x() * -10, pa->get_z() + pa->get_right_z() * -10 };
			pos b = { pa->get_x() + pa->get_right_x() * 10, pa->get_z() + pa->get_right_z() * 10 };
			pos c = { (pa->get_x() + pa->get_right_x() * -10) + pa->get_look_x() * 100,
		   (pa->get_z() + pa->get_right_z() * -10) + pa->get_look_z() * 100, };

			pos d = { pa->get_x() + pa->get_right_x() * 10, pa->get_z() + pa->get_right_z() * 10 };
			pos e = { (pa->get_x() + pa->get_right_x() * 10) + pa->get_look_x() * 100
				, (pa->get_z() + pa->get_right_z() * 10) + pa->get_look_x() * 100 };
			pos f = { (pa->get_x() + pa->get_right_x() * -10) + pa->get_look_x() * 100,
		   (pa->get_z() + pa->get_right_z() * -10) + pa->get_look_z() * 100, };

			pos n = {gaia->boss->get_x(), gaia->boss->get_z()};

			if (isInsideTriangle(a, b, c, n) || isInsideTriangle(d, e, f, n)) {
				pa->set_skill_factor(1, 1);
				float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
				float defence_damage = (gaia->boss->get_defence_factor() *
					gaia->boss->get_magical_defence()) / (1 + (gaia->boss->get_defence_factor() *
						gaia->boss->get_magical_defence()));
				float damage = give_damage * (1 - defence_damage);
				gaia->boss->set_hp(gaia->boss->get_hp() - damage);

				for (int i = 0; i < GAIA_ROOM; ++i)
					send_play_effect_packet(gaia->get_party_palyer()[i], gaia->boss); // 이펙트 터트릴 위치 
			}
		
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 10s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);	
			running_pattern = false;
			break;
		}
		default:
			break;
		}
		break;
	}
	case J_SUPPORTER: {
		switch (p)
		{
		case 0: {  //hp 회복 
			running_pattern = true;
			if (gaia->get_party_palyer()[0]->get_maxhp() == gaia->get_party_palyer()[0]->get_hp() &&
				gaia->get_party_palyer()[1]->get_maxhp() == gaia->get_party_palyer()[1]->get_hp() &&
				gaia->get_party_palyer()[2]->get_maxhp() == gaia->get_party_palyer()[2]->get_hp()) {
				ev.obj_id = pa->get_id();
				ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				timer_queue.push(ev);
				running_pattern = false;
				return;
			}
				

			int tmp_hp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // 낮은 체력 플레이어 찾기 
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
			pa->set_mp(pa->get_mp() - 1000);
			send_buff_ui_packet(gaia->get_party_palyer()[target_player], 2); //ui

			gaia->get_party_palyer()[target_player]->set_hp(gaia->get_party_palyer()[target_player]->get_hp() + gaia->get_party_palyer()[target_player]->get_maxhp() / 10);
			if (gaia->get_party_palyer()[target_player]->get_hp() > gaia->get_party_palyer()[target_player]->get_maxhp()) 
				gaia->get_party_palyer()[target_player]->set_hp(gaia->get_party_palyer()[target_player]->get_maxhp());
			
			for (int i = 0; i < GAIA_ROOM; ++i) {
				send_change_hp_packet(gaia->get_party_palyer()[i], gaia->get_party_palyer()[target_player]);
			}
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_party_palyer()[target_player]->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: { //mp 회복   //여기이상 
			running_pattern = true;
			if (gaia->get_party_palyer()[0]->get_maxmp() == gaia->get_party_palyer()[0]->get_mp() &&
				gaia->get_party_palyer()[1]->get_maxmp() == gaia->get_party_palyer()[1]->get_mp() &&
				gaia->get_party_palyer()[2]->get_maxmp() == gaia->get_party_palyer()[2]->get_mp()) {
				ev.obj_id = pa->get_id();
				ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				timer_queue.push(ev);
				running_pattern = false;
				return;
			}
			int tmp_mp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // 낮은 마나 플레이어 찾기 
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
			pa->set_mp(pa->get_mp() - 1000);
			send_buff_ui_packet(gaia->get_party_palyer()[target_player], 0); //ui

			gaia->get_party_palyer()[target_player]->set_mp(gaia->get_party_palyer()[target_player]->get_mp() + gaia->get_party_palyer()[target_player]->get_maxmp() / 10);
			if (gaia->get_party_palyer()[target_player]->get_mp() > gaia->get_party_palyer()[target_player]->get_maxmp())
				gaia->get_party_palyer()[target_player]->set_mp(gaia->get_party_palyer()[target_player]->get_maxmp());
			for (int i = 0; i < GAIA_ROOM; ++i) {
				send_change_mp_packet(gaia->get_party_palyer()[i], gaia->get_party_palyer()[target_player]); 
			}
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_party_palyer()[target_player]->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: { // 공속 올리기 
			running_pattern = true;
			for (int i = 0; i < GAIA_ROOM; ++i) {
				gaia->get_party_palyer()[i]->attack_speed_up = true;
				send_buff_ui_packet(gaia->get_party_palyer()[i], 4); 
			}
			pa->set_mp(pa->get_mp() - 1000);

			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
			ev.ev = EVENT_PARTNER_SKILL;   // 파트너 (버프) 스킬 이벤트를 따로 만들지 생각해보자 
			ev.target_id = 10;  // 일단 이걸로 구분 
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		default:
			break;
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
		send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
	}

	//hp가 0이되는건 처리 안해놈 
}
void Partner::partner_normal_attack(Partner* pa, Gaia* gaia)
{
	if (running_pattern)
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