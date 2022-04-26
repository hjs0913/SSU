#include "send.h"
#include "Partner.h"
#include <random>
#include <ctime>


Partner::Partner(int d_id) : Player(d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_FREE;
	player_cnt = 0;

	player_rander_ok = 0;
	start_game = false;
	target_id = 0;
	join_dungeon_room = false;
	party_id = new int[GAIA_ROOM];
	running_pattern = false;
	partner_party_id = new int[GAIA_ROOM];
	// Boss Npc Intialize	
	partner = new Player(dungeon_id);
	partner->set_name("ai");

	partner->set_id(-1);
	partner->set_tribe(PARTNER);
	partner->state_lock.lock();
	partner->set_state(ST_FREE);
	partner->state_lock.unlock();




}

Partner::~Partner()
{
	delete party;
}


int Partner::partner_get_id()
{
	return _id;
}

int  Partner::get_dungeon_id()
{
	return dungeon_id;
}

void Partner::set_level(int lv)
{
	level = lv;
}
char* Partner::partner_get_name()
{
	return _name;
}

int Partner::partner_get_lv()
{
	return level;
}
JOB Partner::partner_get_job()
{
	return _job;
}

Partner** Partner::get_party_partner()
{
	return partner_party;
}

DUNGEON_STATE Partner::get_dun_st()
{
	return st;
}

void Partner::set_dun_st(DUNGEON_STATE dst)
{
	st = dst;
}

void Partner::partner_move()
{
	// Raid Map은 장애물이 없으므로 A_star는 낭비다
	if (running_pattern) return;

	if ((party[target_id]->get_x() >= partner->get_x() - 15 && party[target_id]->get_x() <= partner->get_x() + 15) &&
		(party[target_id]->get_z() >= partner->get_z() - 15 && party[target_id]->get_z() <= partner->get_z() + 15)) return;

	pos mv = partner->non_a_star(party[target_id]->get_x(), party[target_id]->get_z(), partner->get_x(), partner->get_z());

	//값을 적용시키고 새로운 좌표를 클라이언트에게 보내주기
	partner->set_x(mv.first);
	partner->set_z(mv.second);

	for (auto pt : party) {
		send_move_packet(pt, partner);
		send_look_packet(pt, partner);
	}
}
void Partner::physical_skill_success(int p_id, int target, float skill_factor)
{
	/*
	float give_damage = partners[p_id]->get_physical_attack() * skill_factor;
	float defence_damage = (dungeons[target]->boss->get_defence_factor() *
		dungeons[target]->boss->get_physical_defence()) / (1 + (dungeons[target]->boss->get_defence_factor() *
			dungeons[target]->boss->get_physical_defence()));
	float damage = give_damage * (1 - defence_damage);
	int target_hp = dungeons[target]->boss->get_hp() - damage;

	cout << "give_damage : " << give_damage << endl;
	cout << "defence_damage : " << defence_damage << endl;
	cout << dungeons[target]->boss->get_defence_factor() *
		dungeons[target]->boss->get_physical_defence() << endl;
	cout << (1 + (dungeons[target]->boss->get_defence_factor() *
		dungeons[target]->boss->get_physical_defence())) << endl;

	cout << p_id << "가 " << damage << "을 " << target << "에게 주었다."
		<< target_hp << "남음" << endl;

	dungeons[target]->boss->set_hp(target_hp);
	if (target_hp <= 0) {
		dungeons[target]->boss->state_lock.lock();
		if (dungeons[target]->boss->get_state() != ST_INGAME) {
			dungeons[target]->boss->state_lock.unlock();
			return;
		}
		dungeons[target]->boss->set_state(ST_DEAD);
		dungeons[target]->boss->state_lock.unlock();
		if (target < NPC_ID_START) {
			partners[p_id]->set_active(false);

			sc_packet_dead packet;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_DEAD;
			packet.attacker_id = p_id;
			reinterpret_cast<Player*>(dungeons[target]->boss)->do_send(sizeof(packet), &packet);


			timer_event ev;
			ev.obj_id = target;
			ev.start_time = chrono::system_clock::now() + 3s;
			ev.ev = EVENT_PLAYER_REVIVE;
			ev.target_id = 0;
			timer_queue.push(ev);
		}
		else {
			dungeons[target]->boss->set_active(false);
			timer_event ev;
			ev.obj_id = target;
			ev.start_time = chrono::system_clock::now() + 30s;
			ev.ev = EVENT_NPC_REVIVE;
			ev.target_id = 0;
			timer_queue.push(ev);

			int get_exp = dungeons[target]->boss->get_lv() * dungeons[target]->boss->get_lv() * 2;
			if (dungeons[target]->boss->get_tribe() == BOSS)
				get_exp = get_exp * 2;
			char mess[MAX_CHAT_SIZE];
			sprintf_s(mess, MAX_CHAT_SIZE, "%s을 죽였습니다, %d의 경험치를 획득합니다",
				dungeons[target]->boss->get_name(), get_exp);
			send_chat_packet(reinterpret_cast<Player*>(partners[p_id]), p_id, mess);

			send_status_change_packet(reinterpret_cast<Player*>(partners[p_id]));

			//
			/*
			int max_exp = 100 * pow(2, (partners[p_id]]->get_lv() - 1));
			if (reinterpret_cast<Player*>(partners[p_id])->get_exp() + get_exp >= max_exp) {
				players[p_id]->set_lv(players[p_id]->get_lv() + 1);
				reinterpret_cast<Player*>(partners[p_id])->
					set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp - max_exp);
				sprintf_s(mess, MAX_CHAT_SIZE, "Level up : %d",
					players[p_id]->get_lv());
				send_chat_packet(reinterpret_cast<Player*>(partners[p_id]), p_id, mess);
				send_status_change_packet(reinterpret_cast<Player*>(partners[p_id]));
			}
			else {
				reinterpret_cast<Player*>(partners[p_id])
					->set_exp(reinterpret_cast<Player*>(partners[p_id])->get_exp() + get_exp);
			}
			send_status_change_packet(reinterpret_cast<Player*>(partners[p_id]));
		}

		unordered_set <int> nearlist;
		for (auto& other : dungeons) {
			if (false == is_near(players[target]->get_id(), other->get_id()))
				continue;
			if (ST_INGAME != other->get_state())
				continue;
			if (other->get_tribe() != HUMAN) break;
			nearlist.insert(other->get_id());
		}
		nearlist.erase(target);

		for (auto other : nearlist) {
			Player* other_player = reinterpret_cast<Player*>(players[other]);
			other_player->vl.lock();
			if (0 != other_player->viewlist.count(target)) {
				other_player->viewlist.erase(target);
				other_player->vl.unlock();
				send_remove_object_packet(other_player, players[target]);
			}
			else other_player->vl.unlock();
		}
	}
	else if (p_id >= NPC_ID_START) {
		send_change_hp_packet(reinterpret_cast<Player*>(players[target]), players[target]);
		reinterpret_cast<Player*>(players[target])->vl.lock();
		for (auto id : reinterpret_cast<Player*>(players[target])->viewlist) {
			if (true == is_npc(id)) continue;
			send_change_hp_packet(reinterpret_cast<Player*>(players[id]), players[target]);
		}
		reinterpret_cast<Player*>(players[target])->vl.unlock();

		char mess[MAX_CHAT_SIZE];
		sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
			players[p_id]->get_name(), players[target]->get_name(), damage);
		//send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

		if (reinterpret_cast<Player*>(players[target])->_auto_hp == false) {
			timer_event ev;
			ev.obj_id = target;
			ev.start_time = chrono::system_clock::now() + 5s;
			ev.ev = EVENT_AUTO_PLAYER_HP;
			ev.target_id = 0;
			timer_queue.push(ev);
			reinterpret_cast<Player*>(players[target])->_auto_hp = true;
		}


		timer_event ev;
		ev.obj_id = p_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_NPC_ATTACK;
		ev.target_id = target;
		timer_queue.push(ev);
	}
	else {
		for (auto& obj : players) {
			if (obj->get_state() != ST_INGAME) continue;
			if (true == is_npc(obj->get_id())) break;   // npc가 아닐때
			if (true == is_near(target, obj->get_id())) {      // 근처에 있을때
				send_change_hp_packet(reinterpret_cast<Player*>(obj), players[target]);
			}
		}

		sc_packet_combat_id packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_COMBAT_ID;
		packet.id = target;
		reinterpret_cast<Player*>(players[p_id])->do_send(sizeof(packet), &packet);

		char mess[MAX_CHAT_SIZE];
		sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
			players[p_id]->get_name(), players[target]->get_name(), damage);
		// send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
	}

		}
	}*/
}


void Partner::partner_attack()  //일반공격 기본, 스킬을 쿨타임 돌때마다 계속 쓰도록 하자 
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen);

	/*
	cout << "p : " << p << endl;

	switch (p % 5) {
	case 0: {
		running_pattern = true;

		ev.obj_id = _id;
		ev.start_time = chrono::system_clock::now() + 10s;
		ev.ev = EVENT_PARTNER_PATTERN;
		ev.target_id = 0;
		timer_queue.push(ev);

		cout << "최후의 일격 !!!" << endl;
		partner->set_mp(partner->get_mp() - 1000);

		for (int i = 0; i <= MAX_USER / GAIA_ROOM; ++i) {
			dungeons[i]->state_lock.lock();
			if (dungeons[i]->get_dun_st() != DUN_ST_START) {
				dungeons[i]->state_lock.unlock();
				continue;
			}

			dungeons[i]->state_lock.unlock();

			if ((dungeons[i]->boss->get_x() >= partners[0]->get_x() - 10 && dungeons[i]->boss->get_x() <= partners[0]->get_x() + 10) && (dungeons[i]->boss->get_z() >= partners[0]->get_z() - 10 && dungeons[i]->boss->get_z() <= partners[0]->get_z() + 10)) {
				partners[0]->set_skill_factor(0, 0);  // 파트너와 가이아와의 스킬 석세스 따로 만들자 
				physical_skill_success(_id, dungeons[i]->get_dungeon_id(), partners[0]->get_skill_factor(0, 0));
				dungeons[i]->boss->set_target_id(partners[0]->partner_get_id());
				send_status_change_packet(partners[0]);
				if (dungeons[i]->boss->get_active() == false && dungeons[i]->boss->get_tribe() == BOSS) {
					dungeons[i]->boss->set_active(true);
					timer_event ev;
					ev.obj_id = i;
					ev.start_time = chrono::system_clock::now() + 3s;
					ev.ev = EVENT_BOSS_ATTACK;
					ev.target_id = -1; //-1
					timer_queue.push(ev);


				}
			}
		}


		break;
	}
	case 1: {
		timer_event ev;
		ev.obj_id = _id;
		ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
		ev.ev = EVENT_PARTNER_PATTERN;
		ev.target_id = 0;
		timer_queue.push(ev);



		Coord_P a = { partners[0]->get_x(), partners[0]->get_z() };    //플레이어 기준 전방 삼각형 범위 
		Coord_P b = { partners[0]->get_x() - partners[0]->get_right_x() * 40 + partners[0]->get_look_x() * 100,
			partners[0]->get_z() - partners[0]->get_right_z() * 40 + partners[0]->get_look_z() * 100 };  // 왼쪽 위
		Coord_P c = { partners[0]->get_x() + partners[0]->get_right_x() * 40 + partners[0]->get_look_x() * 100,
			partners[0]->get_z() + partners[0]->get_right_z() * 40 + partners[0]->get_look_z() * 100 };  // 오른쪽 위

		cout << "광야 일격 !!!" << endl;
		partners[0]->set_mp(partners[0]->get_mp() - 1000);
		for (int i = 0; i <= MAX_USER / GAIA_ROOM; ++i) {
			dungeons[i]->state_lock.lock();
			if (dungeons[i]->get_dun_st() != DUN_ST_START) {
				dungeons[i]->state_lock.unlock();
				continue;
			}
			dungeons[i]->state_lock.unlock();

			Coord_P n = { dungeons[i]->boss->get_x(), dungeons[i]->boss->get_z() };
			float px = dungeons[i]->boss->get_x();
			float pz = dungeons[i]->boss->get_z();

			if (isInsideTriangle(a, b, c, n)) {
				partners[0]->set_skill_factor(1, 0);
				physical_skill_success(_id, dungeons[i]->get_dungeon_id(), partners[0]->get_skill_factor(1, 0));
				dungeons[i]->boss->set_target_id(partners[0]->get_id());
				send_status_change_packet(partners[0]);
				if (dungeons[i]->boss->get_active() == false && dungeons[i]->boss->get_tribe() == BOSS) {
					dungeons[i]->boss->set_active(true);
					timer_event ev;
					ev.obj_id = i;
					ev.start_time = chrono::system_clock::now() + 3s;
					ev.ev = EVENT_BOSS_ATTACK;
					ev.target_id = -1;
					timer_queue.push(ev);
				}
			}
		}


	}
	case 2: {

		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 3: {

		ev.obj_id = dungeon_id;
		ev.start_time = chrono::system_clock::now() + 3s;
		ev.ev = EVENT_BOSS_ATTACK;
		ev.target_id = -1;
		timer_queue.push(ev);
		break;
	}
	case 4: {
		running_pattern = true;


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
	*/
}