#include "Player.h"
#include "send.h"

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
    attack_speed_up = false;
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

void Player::attack_success(Npc* target)
{

}

void Player::revive()
{
    if (join_dungeon_room == true) {
        dungeons[pl->indun_id]->state_lock.lock();
        if (dungeons[pl->indun_id]->get_dun_st() == DUN_ST_START) {
            dungeons[pl->indun_id]->state_lock.unlock();

            pl->state_lock.lock();
            if (pl->get_state() != ST_DEAD) {
                pl->state_lock.unlock();
                return;
            }
            pl->set_state(ST_INDUN);
            pl->state_lock.unlock();

            // 초기화
            pl->set_hp(pl->get_maxhp());
            pl->set_mp(pl->get_maxmp());
            send_status_change_packet(pl);

            // 시야처리
            Player** partys = dungeons[pl->indun_id]->get_party_palyer();
            for (int i = 0; i < 4; i++) {
                if (partys[i]->get_tribe() != HUMAN) continue;
                send_change_hp_packet(partys[i], pl);
                send_revive_packet(partys[i], pl);
            }

            if (pl->get_tribe() == PARTNER) {
                timer_event ev;
                ev.obj_id = pl->get_id();
                ev.start_time = chrono::system_clock::now() + 10s;
                ev.ev = EVENT_PARTNER_MOVE;
                ev.target_id = 1;
                timer_queue.push(ev);

                ev.obj_id = pl->get_id();
                ev.start_time = chrono::system_clock::now() + 1s;
                ev.ev = EVENT_PARTNER_NORMAL_ATTACK;
                ev.target_id = 1;
                timer_queue.push(ev);

                ev.obj_id = pl->get_id();
                ev.start_time = chrono::system_clock::now() + 3s;
                ev.ev = EVENT_PARTNER_SKILL;
                ev.target_id = 1;
                timer_queue.push(ev);
            }

            return;
        }
        dungeons[pl->indun_id]->state_lock.unlock();
    }

    state_lock.lock();
    if (_state != ST_DEAD) {
        state_lock.unlock();
        return;
    }
    _state = ST_INGAME;
    state_lock.unlock();

    // 플레이어 죽은 후 초기화 설정
    _hp = _maxhp;
    _x = 3210;
    _y = 0;
    _z = 940;
    _exp = _exp / 2;
    send_status_change_packet(this);

    send_revive_packet(this, this);
}
