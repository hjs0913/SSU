#include "Player.h"

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

void Player::set_socket(SOCKET c_socket)
{
    _socket = c_socket;
}

void Player::CloseSocketPlayer()
{
    closesocket(_socket);
}
