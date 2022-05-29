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

	switch (pa->get_job()) // AI�� ������ ���� �������� ������ 
	{
	case J_DILLER: {      //������� �ϴ� �������͸� ������ 
		if (gaia->running_pattern == false) {   //���� ������ ���� ���ϸ� �پ�
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if (gaia->running_pattern == true) {  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
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
			case 1:    //  �������� 2�� �� ������ ���� 
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
		if (gaia->running_pattern == false) {   //���� ������ ���� ���ϸ� �پ�
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x(), gaia->boss->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if (gaia->running_pattern == true) {  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
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
			case 1:    //  �������� 2�� �� ������ ���� 
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
		if (gaia->running_pattern == false) {   //���� ������ ���� ���ص� �׷��� �����⸮�� ������   // �ϴ� ������ �ٶ󺸴� �ݴ����?
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() - gaia->boss->get_look_x() * 100, gaia->boss->get_z() + gaia->boss->get_look_z() * 100, pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if (gaia->running_pattern == true) {  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
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
			case 1:    //  �������� 2�� �� ������ ���� 
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
		if (gaia->running_pattern == false) {   //���� ������ ���� ���ص� �׷��� �����⸮�� ������ 
			target_id = gaia->get_dungeon_id();
			pos mv = pa->non_a_star(gaia->boss->get_x() - gaia->boss->get_look_x() * 100, gaia->boss->get_z() + gaia->boss->get_look_z() * 100, pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if (gaia->running_pattern == true) {  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
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
			case 1:    //  �������� 2�� �� ������ ���� 
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

void Partner::partner_attack(Partner* pa, Gaia* gaia) //��ų�� ��Ÿ�� �������� ��� ������ ����     //  ����Ÿ�� �����ΰ�?
{

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen) % 3;

	//�ϴ� ������ ���� �ٽ� �з� �սô�.
	//�׸��� ai�� ��ŷ�� �ʿ���� ���� // **�����̶� hp, mpȮ���� ���� �ʿ��� ������� ���� �ְ� ���� ui��Ŷ�� ������ 
	switch (pa->get_job()) // AI�� ������ ���� �������� ������ 
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
			pos a = { pa->get_x(), pa->get_z() };    //�÷��̾� ���� ���� �ﰢ�� ���� 
			pos b = { pa->get_x() - pa->get_right_x() * 40 + pa->get_look_x() * 100,
				pa->get_z() - pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // ���� ��
			pos c = { pa->get_x() + pa->get_right_x() * 40 + pa->get_look_x() * 100,
				pa->get_z() + pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // ������ ��

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
			ev.start_time = chrono::system_clock::now() + 3s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: {
			running_pattern = true;
			pa->set_mp(pa->get_mp() - 1000);

			pa->set_physical_attack(0.6 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //�ϴ� �ι� 
			pa->set_magical_attack(0.2 * pa->get_lv() * pa->get_lv() + 5 * pa->get_lv());
			//send_status_change_packet(pl);

			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 3s;  //��Ÿ��
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
		case 0: {   //�о�� ���� 
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
			ev.start_time = chrono::system_clock::now() + 3s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_dungeon_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: {  //��׷� ���� 
			running_pattern = true;
	
			pa->set_mp(pa->get_mp() - 1000);
	
			if ((gaia->boss->get_x() >= pa->get_x() - 40 && gaia->boss->get_x() <= pa->get_x() + 40) && (gaia->boss->get_z() >= pa->get_z() - 40 && gaia->boss->get_z() <= pa->get_z() + 40)) {
				pa->set_skill_factor(1, 0);
				gaia->target_id = pa->get_indun_id();
				//send_status_change_packet(pl);
			} 
		
			ev.obj_id = gaia->get_dungeon_id();       // ������ ���߿� �ٽ� 
			ev.start_time = chrono::system_clock::now() + 7s;
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = pa->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: {  //�ڱ� ���� ���� 
			running_pattern = true;

			pa->set_mp(pa->get_mp() - 1000);
			pa->set_physical_defence(0.54 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //�ϴ� �ι� 
			pa->set_magical_defence(0.4 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv());
			//send_status_change_packet(pl);
			
		            // ������ ���߿� �ٽ� 
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 10s;  //��Ÿ��
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
		case 0: {  //�� �� ���̰� ��ų ����� ���� hp�� ��� �� mp�� ä��  
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
		case 1: {  // ���׿�, ��������? 
			running_pattern = true;

			pa->set_mp(pa->get_mp() - 1500);

			for (int i = 0; i < GAIA_ROOM; ++i)    //�̰� �׸���� �����ִ°Ŵ� // �ٵ� partner �������� �Լ� ���� ó������ ****
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
					send_play_effect_packet(gaia->get_party_palyer()[i], gaia->boss); // ����Ʈ ��Ʈ�� ��ġ 
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
		case 0: {  //hp ȸ�� 
			running_pattern = true;
			if (gaia->get_party_palyer()[0]->get_maxhp() == gaia->get_party_palyer()[0]->get_hp() &&
				gaia->get_party_palyer()[1]->get_maxhp() == gaia->get_party_palyer()[1]->get_hp() &&
				gaia->get_party_palyer()[2]->get_maxhp() == gaia->get_party_palyer()[2]->get_hp()) {
				ev.obj_id = pa->get_id();
				ev.start_time = chrono::system_clock::now() + 5s;  //��Ÿ��
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				timer_queue.push(ev);
				running_pattern = false;
				return;
			}
				

			int tmp_hp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // ���� ü�� �÷��̾� ã�� 
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
			ev.start_time = chrono::system_clock::now() + 5s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_party_palyer()[target_player]->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 1: { //mp ȸ��   //�����̻� 
			running_pattern = true;
			if (gaia->get_party_palyer()[0]->get_maxmp() == gaia->get_party_palyer()[0]->get_mp() &&
				gaia->get_party_palyer()[1]->get_maxmp() == gaia->get_party_palyer()[1]->get_mp() &&
				gaia->get_party_palyer()[2]->get_maxmp() == gaia->get_party_palyer()[2]->get_mp()) {
				ev.obj_id = pa->get_id();
				ev.start_time = chrono::system_clock::now() + 5s;  //��Ÿ��
				ev.ev = EVENT_PARTNER_SKILL;
				ev.target_id = 0;
				timer_queue.push(ev);
				running_pattern = false;
				return;
			}
			int tmp_mp = 0;
			int target_player = 0;
			for (int i = 0; i < GAIA_ROOM; ++i) {   // ���� ���� �÷��̾� ã�� 
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
			ev.start_time = chrono::system_clock::now() + 5s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_SKILL;
			ev.target_id = gaia->get_party_palyer()[target_player]->get_id();
			timer_queue.push(ev);
			running_pattern = false;
			break;
		}
		case 2: { // ���� �ø��� 
			running_pattern = true;
			for (int i = 0; i < GAIA_ROOM; ++i) {
				gaia->get_party_palyer()[i]->attack_speed_up = true;
				send_buff_ui_packet(gaia->get_party_palyer()[i], 4); 
			}
			pa->set_mp(pa->get_mp() - 1000);

			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 5s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_SKILL;   // ��Ʈ�� (����) ��ų �̺�Ʈ�� ���� ������ �����غ��� 
			ev.target_id = 10;  // �ϴ� �̰ɷ� ���� 
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

	//hp�� 0�̵Ǵ°� ó�� ���س� 
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