﻿#include "stdafx.h"
#include "Player.h"
#include "database.h"
#include "send.h"
#include <fstream>
#include <queue>
#include <random>
#include "Partner.h"
CRITICAL_SECTION cs;

HANDLE g_h_iocp;
SOCKET g_s_socket;
array <Npc*, MAX_USER + MAX_NPC> players;
array <Gaia*, MAX_USER / GAIA_ROOM> dungeons;
array <Obstacle, MAX_OBSTACLE> obstacles;

concurrency::concurrent_priority_queue<timer_event> timer_queue;

bool is_near(int a, int b)
{
    if (RANGE < abs(players[a]->get_x() - players[b]->get_x())) return false;
    if (RANGE < abs(players[a]->get_z() - players[b]->get_z())) return false;
    return true;
}

bool is_agro_near(int a, int b)
{
    if (players[b]->get_tribe() != BOSS) return false;
    if (AGRORANGE < abs(players[a]->get_x() - players[b]->get_x())) return false;
    if (AGRORANGE < abs(players[a]->get_z() - players[b]->get_z())) return false;
    return true;
}

bool check_move_alright(int x, int z, bool monster)
{
    int size = 0;
    if (monster) size = 15;
    else size = 5;


    for (auto& ob : obstacles) {
        if ((ob.get_x() - size <= x && x <= ob.get_x() + size) && (ob.get_z() - size <= z && z <= ob.get_z() + size)) {
            return false;
        }
    }
    return true;
}

bool is_npc(int id)
{
    return (id >= NPC_ID_START) && (id <= NPC_ID_END);
}

bool is_player(int id)
{
    return (id >= 0) && (id < MAX_USER);
}

// 스크립트 추가
void Activate_Npc_Move_Event(int target, int player_id)
{
    EXP_OVER* exp_over = new EXP_OVER;
    exp_over->_comp_op = OP_NPC_MOVE;
    exp_over->_target = player_id;
    PostQueuedCompletionStatus(g_h_iocp, 1, target, &exp_over->_wsa_over);
}

void magical_skill_success(int p_id, int target, float skill_factor)
{

    float give_damage = players[p_id]->get_magical_attack() * skill_factor;
    float defence_damage = (players[target]->get_defence_factor() *
        players[target]->get_magical_defence()) / (1 + (players[target]->get_defence_factor() *
            players[target]->get_magical_defence()));
    float damage = give_damage * (1 - defence_damage);
    int target_hp = players[target]->get_hp() - damage;

    if (target_hp <= 0) target_hp = 0;
    players[target]->set_hp(target_hp);

    if (target_hp <= 0) {
        players[target]->state_lock.lock();
        if (players[target]->get_state() != ST_INGAME) {
            players[target]->state_lock.unlock();
            return;
        }
        players[target]->set_state(ST_DEAD);
        players[target]->state_lock.unlock();
        if (target < NPC_ID_START) {
            players[p_id]->set_active(false);

            sc_packet_dead packet;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_DEAD;
            packet.id = target;
            packet.attacker_id = p_id;
            reinterpret_cast<Player*>(players[target])->do_send(sizeof(packet), &packet);

            send_notice(reinterpret_cast<Player*>(players[target]), "사망했습니다. 10초 후 부활합니다", 1);

            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 10s;
            ev.ev = EVENT_PLAYER_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);
        }
        else {
            players[target]->set_active(false);
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 30s;
            ev.ev = EVENT_NPC_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);

            int get_exp = players[target]->get_lv() * players[target]->get_lv() * 2;
            if (players[target]->get_tribe() == BOSS)
                get_exp = get_exp * 2;
            char mess[MAX_CHAT_SIZE];
            sprintf_s(mess, MAX_CHAT_SIZE, "%s을 죽였습니다, %d의 경험치를 획득합니다",
                players[target]->get_name(), get_exp);
            send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);

            send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));

            int max_exp = 100 * pow(2, (players[p_id]->get_lv() - 1));
            if (reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp >= max_exp) {
                players[p_id]->set_lv(players[p_id]->get_lv() + 1);
                reinterpret_cast<Player*>(players[p_id])->
                    set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp - max_exp);
                sprintf_s(mess, MAX_CHAT_SIZE, "Level up : %d",
                    players[p_id]->get_lv());
                send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
                send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));
            }
            else {
                reinterpret_cast<Player*>(players[p_id])
                    ->set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp);
            }
            send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));
        }

        unordered_set <int> nearlist;
        for (auto& other : players) {
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
                send_dead_packet(other_player, players[p_id], players[target]);
                //send_remove_object_packet(other_player, players[target]);
            }
            else other_player->vl.unlock();
        }
    }
    else if (p_id >= NPC_ID_START) {

        //send_status_change_packet(reinterpret_cast<Player*>(players[target]));

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
        //send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
    }
}

void physical_skill_success(int p_id, int target, float skill_factor)
{

    float give_damage = players[p_id]->get_physical_attack() * skill_factor;
    float defence_damage = (players[target]->get_defence_factor() *
        players[target]->get_physical_defence()) / (1 + (players[target]->get_defence_factor() *
            players[target]->get_physical_defence()));
    float damage = give_damage * (1 - defence_damage);
    int target_hp = players[target]->get_hp() - damage;

    if (target_hp <= 0) target_hp = 0;
    players[target]->set_hp(target_hp);

    if (target_hp <= 0) {
        players[target]->state_lock.lock();
        if (players[target]->get_state() != ST_INGAME) {
            players[target]->state_lock.unlock();
            return;
        }
        players[target]->set_state(ST_DEAD);
        players[target]->state_lock.unlock();
        if (target < NPC_ID_START) {
            players[p_id]->set_active(false);

            sc_packet_dead packet;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_DEAD;
            packet.attacker_id = p_id;
            reinterpret_cast<Player*>(players[target])->do_send(sizeof(packet), &packet);

            send_notice(reinterpret_cast<Player*>(players[target]), "사망했습니다. 10초 후 부활합니다", 1);

            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 10s;
            ev.ev = EVENT_PLAYER_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);
        }
        else {
            players[target]->set_active(false);
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 30s;
            ev.ev = EVENT_NPC_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);

            int get_exp = players[target]->get_lv() * players[target]->get_lv() * 2;
            if (players[target]->get_tribe() == BOSS)
                get_exp = get_exp * 2;
            char mess[MAX_CHAT_SIZE];
            sprintf_s(mess, MAX_CHAT_SIZE, "%s을 죽였습니다, %d의 경험치를 획득합니다",
                players[target]->get_name(), get_exp);
            send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);

            send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));

            int max_exp = 100 * pow(2, (players[p_id]->get_lv() - 1));
            if (reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp >= max_exp) {
                players[p_id]->set_lv(players[p_id]->get_lv() + 1);
                reinterpret_cast<Player*>(players[p_id])->
                    set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp - max_exp);
                sprintf_s(mess, MAX_CHAT_SIZE, "Level up : %d",
                    players[p_id]->get_lv());
                send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
                send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));
            }
            else {
                reinterpret_cast<Player*>(players[p_id])
                    ->set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp() + get_exp);
            }
            send_status_change_packet(reinterpret_cast<Player*>(players[p_id]));
        }

        unordered_set <int> nearlist;
        for (auto& other : players) {
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
                send_dead_packet(other_player, players[p_id], players[target]);
                //send_remove_object_packet(other_player, players[target]);
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

struct Coord
{
    float x;
    float z;
};

bool check_inside(Coord a, Coord b, Coord c, Coord n) {
    Coord A, B, C;
    A.x = b.x - a.x;
    A.z = b.z - a.z;
    B.x = c.x - a.x;
    B.z = c.z - a.z;
    C.x = n.x - a.x;
    C.z = n.z - a.z;

    if ((A.x * B.z - A.z * B.x) * (A.x * C.z - A.z * C.x) < 0)
        return false;
    return true;
}

bool isInsideTriangle(Coord a, Coord b, Coord c, Coord n)
{
    if (!check_inside(a, b, c, n)) return false;
    if (!check_inside(b, c, a, n)) return false;
    if (!check_inside(c, a, b, n)) return false;
    return true;

}

COMP_OP EVtoOP(EVENT_TYPE ev) {
    switch (ev)
    {
    case EVENT_NPC_MOVE:
        return OP_NPC_MOVE;
        break;
    case EVENT_NPC_ATTACK: {
        return OP_NPC_ATTACK;
        break;
    }
    case EVENT_AUTO_PLAYER_HP:
        return OP_AUTO_PLAYER_HP;
        break;
    case EVENT_PLAYER_REVIVE:
        return OP_PLAYER_REVIVE;
        break;
    case EVENT_NPC_REVIVE:
        return OP_NPC_REVIVE;
        break;
    case EVENT_BOSS_MOVE:
        return OP_BOSS_MOVE;
        break;
    case EVENT_BOSS_ATTACK:
        return OP_BOSS_ATTACK;
        break;

    case EVENT_ELEMENT_COOLTIME:
        // return OP_ELEMENT_COOLTIME;
        break;
    case EVENT_GAIA_PATTERN:
        return OP_GAIA_PATTERN;
        break;
    case EVENT_PARTNER_MOVE:
        return OP_PARTNER_MOVE;
        break;
    case EVENT_PARTNER_SKILL:
        return OP_PARTNER_SKILL;
        break;
    case EVENT_PARTNER_NORMAL_ATTACK:
        return OP_PARTNER_NORMAL_ATTACK;
        break;
    case EVENT_GAMESTART_TIMER:
        return OP_GAMESTART_TIMER;
        break;
    case EVENT_FINISH_RAID:
        return OP_FINISH_RAID;
        break;
    }

}

void do_timer()
{
    chrono::system_clock::duration dura;
    const chrono::milliseconds waittime = 10ms;
    timer_event temp;
    bool temp_bool = false;
    while (true) {
        if (temp_bool) {
            temp_bool = false;
            if (temp.ev == EVENT_PLAYER_ATTACK) {
                reinterpret_cast<Player*>(players[temp.obj_id])->set_attack_active(false);
            }
            else if (temp.ev == EVENT_SKILL_COOLTIME) {
                if (temp.target_id == 2) {  // 전사 BUFF
                    switch (reinterpret_cast<Player*>(players[temp.obj_id])->get_job())
                    {
                    case J_DILLER: {
                        players[temp.obj_id]->set_physical_attack(0.3 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                        players[temp.obj_id]->set_magical_attack(0.1 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 5 * players[temp.obj_id]->get_lv());
                        break;
                    }
                    case J_TANKER: {
                        players[temp.obj_id]->set_physical_defence(0.27 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                        players[temp.obj_id]->set_magical_defence(0.2 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                        break;
                    }
                    case J_MAGICIAN: {  //없음 

                        break;
                    }
                    case J_SUPPORTER: {   // 대상이 여러명일 때는 어떻게 다시 초기화할까 
                        if (dungeons[temp.obj_id]->start_game == false) {
                            for (int i = 0; i < MAX_USER; ++i) {
                                reinterpret_cast<Player*>(players[i])->attack_speed_up = false;
                            }
                        }
                        else {
                            for (int i = 0; i < GAIA_ROOM; ++i) {
                                dungeons[temp.obj_id]->get_party_palyer()[i]->attack_speed_up = false;
                            }
                        }
                        break;
                    }
                    }
                    // 일단 이것을 넣으면 안돌아감(이유 모름)
                    //send_status_change_packet(reinterpret_cast<Player*>(players[ev.obj_id]));
                }
                reinterpret_cast<Player*>(players[temp.obj_id])->set_skill_active(temp.target_id, false);
                continue;
            }
        /*    else if (temp.ev == EVENT_PARTNER_SKILL) {
                // obj는 가이아의 넘버,  taget은 파트너 자신  
                switch (reinterpret_cast<Player*>(dungeons[temp.obj_id]->get_party_palyer()[temp.target_id])->get_job())
                {
                case J_DILLER: {
                    if (temp.obj_id != MAX_USER / GAIA_ROOM + 1) {
                        dungeons[temp.obj_id]->get_party_palyer()[temp.target_id]->set_physical_attack(0.3 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                        dungeons[temp.obj_id]->get_party_palyer()[temp.target_id]->set_magical_attack(0.1 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 5 * players[temp.obj_id]->get_lv());
                    }
                    break;
                }
                case J_TANKER: {
                    if (temp.obj_id != MAX_USER / GAIA_ROOM + 1) {
                        dungeons[temp.obj_id]->get_party_palyer()[temp.target_id]->set_physical_defence(0.27 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                        dungeons[temp.obj_id]->get_party_palyer()[temp.target_id]->set_magical_defence(0.2 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                    }
                    break;
                }
                case J_SUPPORTER: {   
                    if (temp.target_id == MAX_USER / GAIA_ROOM + 1) {
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            dungeons[temp.obj_id]->get_party_palyer()[i]->attack_speed_up = false;
                        }
                    }
                    break;
                }
                }

            }*/
            else {
                EXP_OVER* ex_over = new EXP_OVER;
                ex_over->_comp_op = EVtoOP(temp.ev);
                ex_over->_target = temp.target_id;
                PostQueuedCompletionStatus(g_h_iocp, 1, temp.obj_id, &ex_over->_wsa_over);   //0은 소켓취급을 받음
            }
        }

        while (true) {
            timer_event ev;
            if (timer_queue.size() == 0) continue;
            timer_queue.try_pop(ev);

            dura = ev.start_time - chrono::system_clock::now();
            if (dura <= 0ms) {
                EXP_OVER* ex_over = new EXP_OVER;
                if (ev.ev == EVENT_PLAYER_ATTACK) {
                    reinterpret_cast<Player*>(players[ev.obj_id])->set_attack_active(false);
                    continue;
                }
                else if (ev.ev == EVENT_SKILL_COOLTIME) {
                    if (ev.target_id == 2) {  // 전사 BUFF
                        switch (reinterpret_cast<Player*>(players[ev.obj_id])->get_job())
                        {
                        case J_DILLER: {
                            players[ev.obj_id]->set_physical_attack(0.3 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                            players[ev.obj_id]->set_magical_attack(0.1 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 5 * players[ev.obj_id]->get_lv());
                            break;
                        }
                        case J_TANKER: {
                            players[ev.obj_id]->set_physical_defence(0.27 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                            players[ev.obj_id]->set_magical_defence(0.2 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                            break;
                        }
                        case J_MAGICIAN: {  //없음 

                            break;
                        }
                        case J_SUPPORTER: {   // 대상이 여러명일 때는 어떻게 다시 초기화할까 
                            if (dungeons[ev.obj_id]->start_game == false) {
                                for (int i = 0; i < MAX_USER; ++i) {
                                    reinterpret_cast<Player*>(players[i])->attack_speed_up = false;
                                }
                            }
                            else {
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    dungeons[ev.obj_id]->get_party_palyer()[i]->attack_speed_up = false;
                                }
                            }
                            break;
                        }
                        }
                        // 일단 이것을 넣으면 안돌아감(이유 모름)
                        //send_status_change_packet(reinterpret_cast<Player*>(players[ev.obj_id]));
                    }           
                    reinterpret_cast<Player*>(players[ev.obj_id])->set_skill_active(ev.target_id, false);
                    continue;
                }
                /*   else if (ev.ev == EVENT_PARTNER_SKILL) {
                    // obj는 가이아의 넘버,  taget은 파트너 자신  
                    switch (reinterpret_cast<Player*>(dungeons[ev.obj_id]->get_party_palyer()[ev.target_id])->get_job())
                    {
                    case J_DILLER: {
                        if (ev.obj_id != MAX_USER / GAIA_ROOM + 1) {
                            dungeons[ev.obj_id]->get_party_palyer()[ev.target_id]->set_physical_attack(0.3 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                            dungeons[ev.obj_id]->get_party_palyer()[ev.target_id]->set_magical_attack(0.1 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 5 * players[ev.obj_id]->get_lv());
                        }
                        break;
                    }
                    case J_TANKER: {
                        if (ev.obj_id != MAX_USER / GAIA_ROOM + 1) {
                            dungeons[ev.obj_id]->get_party_palyer()[ev.target_id]->set_physical_defence(0.27 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                            dungeons[ev.obj_id]->get_party_palyer()[ev.target_id]->set_magical_defence(0.2 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                        }
                        break;
                    }
                    case J_SUPPORTER: {   // 대상이 여러명일 때는 어떻게 다시 초기화할까 
                        if (ev.target_id == MAX_USER / GAIA_ROOM + 1) {
                            for (int i = 0; i < GAIA_ROOM; ++i) {
                                dungeons[ev.obj_id]->get_party_palyer()[i]->attack_speed_up = false;
                            }
                           }
                        break;
                    }
                    }

                }*/

                ex_over->_comp_op = EVtoOP(ev.ev);
                ex_over->_target = ev.target_id;
                PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);   //0은 소켓취급을 받음
            }
            else if (dura <= waittime) {
                temp = ev;
                temp_bool = true;
                break;
            }
            else {
                timer_queue.push(ev);   // 타이머 큐에 넣지 않고 최적화 필요
            }
        }
        this_thread::sleep_for(dura);
        // 쭉 사여있어서 계속 처리를 하도록 해야함
    }
}


int main()
{
    // DB 연결1
    // Initialise_DB();

    thread timer_thread{ do_timer };
    for (int i = 0; i < 16; ++i)
        worker_threads.emplace_back(worker);


    timer_thread.join();
    for (auto& pl : players) {
        if (pl->get_tribe() != HUMAN) break;
        if (ST_INGAME == pl->get_state())
            Disconnect(pl->get_id());
    }

    // DB 연결
    // Disconnect_DB();
}
