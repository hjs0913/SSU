#include "TimerManager.h"
#include "Player.h"
#include "send.h"
#include "Gaia.h"

Player::Player(int id) : Npc(id)
{
    _state = ST_FREE;
    _prev_size = 0;
    _x = 2100;
    _y = 0;
    _z = 1940;
    set_tribe(HUMAN);
    _lv = 25;
    _job = J_DILLER;
    _element = E_NONE;
    _attack_active = false;
    last_move_time = 0;
    superposition = false;
    join_dungeon_room = false;
    indun_id = -1;
    attack_speed_up = 0;
}

void Player::set_exp(int exp)
{
	_exp = exp;
}

void Player::set_job(JOB job)
{
    _job = job;
}

void Player::set_indun_id(int id)
{
    indun_id = id;
}

int Player::get_exp()
{
	return _exp;
}

JOB Player::get_job()
{
    return _job;
}

int Player::get_indun_id()
{
    return indun_id;
}

int Player::get_prev_size()
{
    return _prev_size;
}

void Player::set_prev_size(int prev_size)
{
    _prev_size = prev_size;
}

void Player::accept_initialize()
{
    _prev_size = 0;
    _recv_over._comp_op = OP_RECV;
    _recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf);
    _recv_over._wsa_buf.len = sizeof(_recv_over._net_buf);
    ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
}

void Player::set_socket(SOCKET c_socket)
{
    _socket = c_socket;
}

void Player::CloseSocketPlayer()
{
    closesocket(_socket);
}

void Player::attack_dead_judge(Npc* target, float fDamage)
{
	int target_hp = target->get_hp();
	send_change_hp_packet(this, target, fDamage);

	if (target_hp <= 0) {
		target->state_lock.lock();
		if (target->get_state() != ST_INGAME) {
			target->state_lock.unlock();
			return;
		}
		target->set_state(ST_DEAD);
		target->state_lock.unlock();

		target->set_active(false);
		target->set_move_active(false);
		timer_event ev;
		ev.obj_id = target->get_id();
		ev.start_time = chrono::system_clock::now() + 30s;
		ev.ev = EVENT_NPC_REVIVE;
		ev.target_id = 0;
		TimerManager::timer_queue.push(ev);

		// 플레이어에게 경험치 제공, 그리고 바뀐 경험치와 레벨을 보내주자
		float get_exp = target->get_lv() * target->get_lv() * 2;
		if (target->get_tribe() == BOSS)
			get_exp = get_exp * 2;
		char mess[MAX_CHAT_SIZE];
		sprintf_s(mess, MAX_CHAT_SIZE, "%s을(를) 죽였습니다, %d의 경험치를 획득합니다",
			target->get_name(), (int)get_exp);
		send_chat_packet(this, _id, mess);

		send_status_change_packet(this);

		float max_exp = 100 * pow((_lv -1 ),2 );
		if (_exp + get_exp >= max_exp) {
			_lv += 1;
			_exp = _exp + get_exp - max_exp;
			sprintf_s(mess, MAX_CHAT_SIZE, "Level up : %d",_lv);
			send_chat_packet(this, _id, mess);
		}
		else {
			_exp = _exp + get_exp;
		}
		send_status_change_packet(this);



	}
	else {
		//if (target->get_id() == 1180) { // 던전상황
		//	Player* pl = reinterpret_cast<Player*>(p);
		//	Player** party_player = dungeons[pl->get_indun_id()]->get_party_palyer();
		//	for (int i = 0; i < GAIA_ROOM; i++) {
		//		send_change_hp_packet(reinterpret_cast<Player*>(party_player[i]), target);
		//	}
		//}
		//else {
		//	for (auto& obj : players) {
		//		if (obj->get_state() != ST_INGAME) continue;
		//		if (true == is_npc(obj->get_id())) break;   // npc가 아닐때
		//		if (true == is_near(target->get_id(), obj->get_id())) {      // 근처에 있을때
		//			send_change_hp_packet(reinterpret_cast<Player*>(obj), target);
		//		}
		//	}
		//}
		sc_packet_combat_id packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_COMBAT_ID;
		packet.id = target->get_id();
		do_send(sizeof(packet), &packet);
	}

}

void Player::attack_element_judge(Npc* target)
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
				// Npc에는 없는 속성
				attack_speed_up = 1;
				//공속 상승 
				target->set_element_cooltime(true);
			}
			break;
		case E_FIRE:
			if (target->get_element() == E_ICE || target->get_element() == E_TREE
				|| target->get_element() == E_FULLMETAL) {
				target->set_element_cooltime(true);
				//10초 공격력 10프로의 화상 피해 
				/*
				timer_event ev;
				//if (reinterpret_cast<Player*>(target)->burn_on == false) {
					ev.obj_id = _id;
					ev.start_time = chrono::system_clock::now() + 5s;
					ev.ev = EVENT_ELEMENT_FIRE_COOLTIME;
					ev.target_id = target->get_id();
					TimerManager::timer_queue.push(ev);
					target->set_element_cooltime(true);*/
				//}
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
				//10초동안 공속 감소  or move 보내는 곳에 변수하나 넣어서 이동 못하게 
				attack_speed_up = -1;
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

void Player::basic_attack_success(Npc* target)
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

	attack_dead_judge(target, damage);
}

void Player::phisical_skill_success(Npc* target, float skill_factor)
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

	attack_dead_judge(target, damage);
}

void Player::magical_skill_success(Npc* target, float skill_factor)
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

	attack_dead_judge(target, damage);
}

void Player::revive()
{
    state_lock.lock();
    if (_state != ST_DEAD) {
        state_lock.unlock();
        return;
    }
    _state = ST_INGAME;
    state_lock.unlock();
	_auto_hp = false;
    // 플레이어 죽은 후 초기화 설정
	if (strcmp(_login_id, "admin") == 0) {
		// stress Test teleport
		_hp = _maxhp;
		_x = rand() % 4000;
		_y = 0;
		_z = rand() % 4000;
	}
	else {
		_hp = _maxhp;
		_x = 3210;
		_y = 0;
		_z = 940;
		_exp = _exp / 2;
	}
    send_status_change_packet(this);

    send_revive_packet(this, this);
}

void Player::revive_indun(Gaia* dun)
{
    dun->state_lock.lock();
    if (dun->get_dun_st() == DUN_ST_START) {
        dun->state_lock.unlock();

        state_lock.lock();
        if (get_state() != ST_DEAD) {
            state_lock.unlock();
            return;
        }
        set_state(ST_INDUN);
        state_lock.unlock();

        // 초기화
        _hp = _maxhp;
        _mp = _maxmp;
        send_status_change_packet(this);

        // 시야처리
        Player** partys = dun->get_party_palyer();
        for (int i = 0; i < 4; i++) {
            if (partys[i]->get_tribe() != HUMAN) continue;
            send_change_hp_packet(partys[i], this, 0);
            send_revive_packet(partys[i], this);
        }

        if (_tribe == PARTNER) {
            timer_event ev;
            ev.obj_id = _id;
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_PARTNER_MOVE;
            ev.target_id = 1;
			TimerManager::timer_queue.push(ev);

            ev.obj_id = _id;
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_PARTNER_NORMAL_ATTACK;
            ev.target_id = 1;
			TimerManager::timer_queue.push(ev);

            ev.obj_id = _id;
            ev.start_time = chrono::system_clock::now() + 3s;
            ev.ev = EVENT_PARTNER_SKILL;
            ev.target_id = 1;
			TimerManager::timer_queue.push(ev);
        }

        return;
    }
    dun->state_lock.unlock();
    revive();
}
