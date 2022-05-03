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
	switch (pa->get_job()) // AI�� ������ ���� �������� ������ 
	{
	cout << "�̵�!" << endl;
	case J_DILLER: {      //������� �ϴ� �������͸� ������ 
		if (gaia->running_pattern == false) {   //���� ������ ���� ���ϸ� �پ�
			target_id = get_indun_id();
			pos mv = pa->non_a_star(gaia->get_x(), gaia->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if(gaia->running_pattern == true){  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
			switch (gaia->pattern_num)
			{
			case 0:
				cout << "���� 0������!" << endl;
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
				cout << "���� 1������!" << endl;
				for (int i = 0; i < 2; i++) {
					int x = gaia->pattern_two_safe_zone[i].first;
					int z = gaia->pattern_two_safe_zone[i].second;
					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}
				}
				move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first , gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());

				pa->set_x(move.first);
				pa->set_z(move.second);

				break;
			case 4:
				move = pa->non_a_star(reinterpret_cast<Npc*>(gaia)->get_look_x() + reinterpret_cast<Npc*>(gaia)->get_right_x() * 10, reinterpret_cast<Npc*>(gaia)->get_look_z() + reinterpret_cast<Npc*>(gaia)->get_right_z() * 10, pa->get_x(), pa->get_z());
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
			target_id = get_indun_id();
			pos mv = pa->non_a_star(gaia->get_x(), gaia->get_z(), pa->get_x(), pa->get_z());
			pa->set_x(mv.first);
			pa->set_z(mv.second);
		}
		else if (gaia->running_pattern == true) {  //������ ������ �� ��,  ������ ��ȣ�� �޾Ƽ� ���� �� �ֵ��� ���� // 0.����4�� 1.���ƿ���3��  4.����1�� 
			switch (gaia->pattern_num)
			{
			case 0:
				cout << "���� 0������!" << endl;
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
				cout << "���� 1������!" << endl;
				for (int i = 0; i < 2; i++) {
					int x = gaia->pattern_two_safe_zone[i].first;
					int z = gaia->pattern_two_safe_zone[i].second;
					if (dis < sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z))) {
						dis = sqrt((pa->get_x() - x) * (pa->get_x() - x) + (pa->get_z() - z) * (pa->get_z() - z));
						nearest_num = i;
					}
				}
				move = pa->non_a_star(gaia->pattern_two_safe_zone[nearest_num].first, gaia->pattern_two_safe_zone[nearest_num].second, pa->get_x(), pa->get_z());
				pa->set_x(move.first);
				pa->set_z(move.second);

				break;
			case 4:
				move = pa->non_a_star(reinterpret_cast<Npc*>(gaia)->get_look_x() + reinterpret_cast<Npc*>(gaia)->get_right_x() * 10, reinterpret_cast<Npc*>(gaia)->get_look_z() + reinterpret_cast<Npc*>(gaia)->get_right_z() * 10, pa->get_x(), pa->get_z());
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
		break;
	}
	case J_SUPPORTER: {
		break;
	}
	default:
		break;
	}
	
	

}
void Partner::physical_skill_success(int p_id, int target, float skill_factor)
{

}

void Partner::partner_attack(Partner* pa, Gaia* gaia) //�Ϲݰ��� �⺻, ��ų�� ��Ÿ�� �������� ��� ������ ���� 
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen) % 4;


	switch (p) { // �ø����� �ø���
	case 0: {
		cout << "������ �ϰ� !!!" << endl;
		pa->set_mp(pa->get_mp() - 1000);
		if ((gaia->boss->get_x() >= pa->get_x() - 10 && gaia->boss->get_x() <= pa->get_x() + 10) && (gaia->boss->get_z() >= pa->get_z() - 10 && gaia->boss->get_z() <= pa->get_z() + 10)) {
			cout << "Ÿ�� !!!" << endl;
			pa->set_skill_factor(0, 0);

			float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
			gaia->boss->set_hp(gaia->boss->get_hp() - give_damage);

			for (int i = 0; i < GAIA_ROOM; ++i) {
				send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
			}

			ev.obj_id = 1;
			ev.start_time = chrono::system_clock::now() + 10s;
			ev.ev = EVENT_PARTNER_ATTACK;
			ev.target_id = -1;
			timer_queue.push(ev);
			break;
		}

		break;
	}
	case 1: {
		pos a = { pa->get_x(), pa->get_z() };    //�÷��̾� ���� ���� �ﰢ�� ���� 
		pos b = { pa->get_x() - pa->get_right_x() * 40 + pa->get_look_x() * 100,
			pa->get_z() - pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // ���� ��
		pos c = { pa->get_x() + pa->get_right_x() * 40 + pa->get_look_x() * 100,
			pa->get_z() + pa->get_right_z() * 40 + pa->get_look_z() * 100 };  // ������ ��

		cout << "���� �ϰ� !!!" << endl;
		pa->set_mp(pa->get_mp() - 1000);

		pos n = { gaia->boss->get_x(),gaia->boss->get_z() };


		if (isInsideTriangle(a, b, c, n)) {
			cout << "Ÿ�� !!!" << endl;
			pa->set_skill_factor(1, 0);
			float give_damage = pa->get_magical_attack() * pa->get_skill_factor(1, 0);
			gaia->boss->set_hp(gaia->boss->get_hp() - give_damage);

			for (int i = 0; i < GAIA_ROOM; ++i) {
				send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
			}

			timer_event ev;
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 10s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_ATTACK;
			ev.target_id = 1;
			timer_queue.push(ev);

		}
		break;
	}
	case 2: {
		cout << "�Ʒ����� ��ȣ !!!" << endl;
		pa->set_mp(pa->get_mp() - 1000);

		pa->set_physical_attack(0.6 * pa->get_lv() * pa->get_lv() + 10 * pa->get_lv()); //�ϴ� �ι� 
		pa->set_magical_attack(0.2 * pa->get_lv() * pa->get_lv() + 5 * pa->get_lv());
		//send_status_change_packet(pl);

		timer_event ev;
		ev.obj_id = pa->get_id();
		ev.start_time = chrono::system_clock::now() + 10s;  //��Ÿ��
		ev.ev = EVENT_PARTNER_ATTACK;
		ev.target_id = 2;
		timer_queue.push(ev);


		break;
	}
	case 3: {
		cout << "�о�� !!!" << endl;
		pa->set_mp(pa->get_mp() - 1000);

		if ((gaia->boss->get_x() >= pa->get_x() - 15 && gaia->boss->get_x() <= pa->get_x() + 15) && (gaia->boss->get_z() >= pa->get_z() - 15 && gaia->boss->get_z() <= pa->get_z() + 15)) {
			cout << "Ÿ�� !!!" << endl;
			pa->set_skill_factor(0, 0);
			float give_damage = pa->get_physical_attack() * pa->get_skill_factor(0, 0);
			gaia->boss->set_pos(gaia->boss->get_x() + pa->get_look_x() * 40, gaia->boss->get_z() + pa->get_look_z() * 40);
			for (int i = 0; i < GAIA_ROOM; ++i) {
				send_move_packet(gaia->get_party_palyer()[i], gaia->boss);
				send_change_hp_packet(gaia->get_party_palyer()[i], gaia->boss);
			}

			timer_event ev;
			ev.obj_id = pa->get_id();
			ev.start_time = chrono::system_clock::now() + 10s;  //��Ÿ��
			ev.ev = EVENT_PARTNER_ATTACK;
			ev.target_id = 0;
			timer_queue.push(ev);
			break;

		}
	}
	default:
		cout << "���� ����" << endl;
		break;
	}

}