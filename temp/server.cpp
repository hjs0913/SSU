#include "stdafx.h"
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

void do_npc_move(int npc_id, int target);
void return_npc_position(int npc_id);

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

int get_new_id()
{
    static int g_id = 0;

    for (int i = 0; i < MAX_USER; ++i) {
        players[i]->state_lock.lock();
        if (ST_FREE == players[i]->get_state()) {
            players[i]->set_state(ST_ACCEPT);
            players[i]->state_lock.unlock();
            return i;
        }
        players[i]->state_lock.unlock();
    }
    cout << "Maximum Number of Clients Overflow!!\n";
    return -1;
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

void attack_success(Npc* p, Npc* target, float atk_factor)
{
    // 현재 물리 공격에 대해서만 생각한다
    float give_damage = p->get_physical_attack() * atk_factor;
    float defence_damage = (target->get_defence_factor() *
        target->get_physical_defence()) / (1 + (target->get_defence_factor() *
            target->get_physical_defence()));
    float damage = give_damage * (1 - defence_damage);
    int target_hp = target->get_hp() - damage;

    if (target_hp <= 0) target_hp = 0;
    target->set_hp(target_hp);

    //timer_event ev;
    //ev.obj_id = p_id;
    //ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
    //ev.ev = EVENT_ELEMENT_COOLTIME;;
    //ev.target_id = target;
    //timer_queue.push(ev);


    if (target->get_element_cooltime() == false) {
        switch (p->get_element())
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
                reinterpret_cast<Player*>(p)->set_physical_defence(reinterpret_cast<Player*>(p)->get_physical_defence() + reinterpret_cast<Player*>(p)->get_physical_defence() / 10);
                target->set_element_cooltime(true);
            }
            break;
        case E_WIND:
            if (target->get_element() == E_WATER || target->get_element() == E_EARTH
                || target->get_element() == E_FIRE) {
                reinterpret_cast<Player*>(p)->attack_speed_up = true;
                //공속  상승 , 쿨타임 감소 
                target->set_element_cooltime(true);
            }
            break;
        case E_FIRE:
            if (target->get_element() == E_ICE || target->get_element() == E_TREE
                || target->get_element() == E_FULLMETAL) {
                //10초 공격력 10프로의 화상 피해 
                target->set_element_cooltime(true);
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
                reinterpret_cast<Player*>(p)->set_magical_defence(reinterpret_cast<Player*>(p)->get_magical_defence() + reinterpret_cast<Player*>(p)->get_magical_defence() / 10);
                target->set_element_cooltime(true);
            }
            break;
        case E_ICE:
            if (target->get_element() == E_TREE || target->get_element() == E_WATER
                || target->get_element() == E_WIND) {
                //동결 and  10초동안 공속, 시전속도, 이동속도 10프로감소 
                target->set_element_cooltime(true);
            }
            break;
        default:
            break;
        }
        if (target->get_element_cooltime() == true) {
            timer_event ev;
            ev.obj_id = p->get_id();
            ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
            ev.ev = EVENT_ELEMENT_COOLTIME;;
            ev.target_id = target->get_id();
            timer_queue.push(ev);
        }
    }


    //  EXP_OVER* exp_over = new EXP_OVER;
     // exp_over->_comp_op = OP_ELEMENT_COOLTIME;
    //  exp_over->_target = target;
    //  PostQueuedCompletionStatus(g_h_iocp, p_id, target, &exp_over->_wsa_over);


    if (target_hp <= 0) {
        target->state_lock.lock();
        if (target->get_state() != ST_INGAME) {
            target->state_lock.unlock();
            return;
        }
        target->set_state(ST_DEAD);
        target->state_lock.unlock();
        if (target->get_id() < NPC_ID_START) {    // 죽은게 플레이어이다
            if (target->get_tribe() == BOSS) return;    // 보스 죽음
            p->set_active(false);
            // 죽은것이 플레이어라면 죽었다는 패킷을 보내준다
            sc_packet_dead packet;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_DEAD;
            packet.id = target->get_id();
            packet.attacker_id = p->get_id();
            reinterpret_cast<Player*>(target)->do_send(sizeof(packet), &packet);

            send_notice(reinterpret_cast<Player*>(target), "사망했습니다. 10초 후 부활합니다", 1);

            // 3초후 부활하며 부활과 동시에 위치 좌표를 수정해준다
            timer_event ev;
            ev.obj_id = target->get_id();
            ev.start_time = chrono::system_clock::now() + 10s;
            ev.ev = EVENT_PLAYER_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);
        }
        else {  // NPC라면 30초 후에 부활할 수 있도록 하자
            target->set_active(false);
            timer_event ev;
            ev.obj_id = target->get_id();
            ev.start_time = chrono::system_clock::now() + 30s;
            ev.ev = EVENT_NPC_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);

            // 플레이어에게 경험치 제공, 그리고 바뀐 경험치와 레벨을 보내주자
            int get_exp = target->get_lv() * target->get_lv() * 2;
            if (target->get_tribe() == BOSS)
                get_exp = get_exp * 2;
            char mess[MAX_CHAT_SIZE];
            sprintf_s(mess, MAX_CHAT_SIZE, "%s을 죽였습니다, %d의 경험치를 획득합니다",
                target->get_name(), get_exp);
            send_chat_packet(reinterpret_cast<Player*>(p), p->get_id(), mess);

            send_status_change_packet(reinterpret_cast<Player*>(p));

            int max_exp = 100 * pow(2, (p->get_lv() - 1));
            if (reinterpret_cast<Player*>(p)->get_exp() + get_exp >= max_exp) {
                p->set_lv(p->get_lv() + 1);
                reinterpret_cast<Player*>(p)->
                    set_exp(reinterpret_cast<Player*>(p)->get_exp() + get_exp - max_exp);
                sprintf_s(mess, MAX_CHAT_SIZE, "Level up : %d",
                    p->get_lv());
                send_chat_packet(reinterpret_cast<Player*>(p), p->get_id(), mess);
                send_status_change_packet(reinterpret_cast<Player*>(p));
            }
            else {
                reinterpret_cast<Player*>(p)
                    ->set_exp(reinterpret_cast<Player*>(p)->get_exp() + get_exp);
            }
            send_status_change_packet(reinterpret_cast<Player*>(p));
        }
        // 죽은 target 주위의 플레이어에게 사라지게 해주자
        unordered_set <int> nearlist;
        for (auto& other : players) {
            if (false == is_near(target->get_id(), other->get_id()))
                continue;
            if (ST_INGAME != other->get_state())
                continue;
            if (other->get_tribe() != HUMAN) break;
            nearlist.insert(other->get_id());
        }
        nearlist.erase(target->get_id());

        for (auto other : nearlist) {
            Player* other_player = reinterpret_cast<Player*>(players[other]);
            other_player->vl.lock();
            if (0 != other_player->viewlist.count(target->get_id())) {
                other_player->viewlist.erase(target->get_id());
                other_player->vl.unlock();
                send_dead_packet(other_player, p, target);
                // send_remove_object_packet(other_player, target);
            }
            else other_player->vl.unlock();
        }
    }
    else if (p->get_id() >= NPC_ID_START) {
        // 플레이어가 공격을 당한 것이므로 hp정보가 바뀌었으므로 그것을 보내주자
        // send_status_change_packet(reinterpret_cast<Player*>(players[target]));

        // 플레이어의 ViewList에 있는 플레이어들에게 보내주자
        send_change_hp_packet(reinterpret_cast<Player*>(target), target);
        reinterpret_cast<Player*>(target)->vl.lock();
        for (auto id : reinterpret_cast<Player*>(target)->viewlist) {
            if (true == is_npc(id)) continue;
            send_change_hp_packet(reinterpret_cast<Player*>(players[id]), target);
        }
        reinterpret_cast<Player*>(target)->vl.unlock();


        char mess[MAX_CHAT_SIZE];
        //send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

        // hp가 깎이였으므로 hp자동회복을 해주도록 하자
        if (reinterpret_cast<Player*>(target)->_auto_hp == false) {
            timer_event ev;
            ev.obj_id = target->get_id();
            ev.start_time = chrono::system_clock::now() + 5s;
            ev.ev = EVENT_AUTO_PLAYER_HP;
            ev.target_id = 0;
            timer_queue.push(ev);
            reinterpret_cast<Player*>(target)->_auto_hp = true;
        }

        // npc공격이면 타이머 큐에 다시 넣어주자
        timer_event ev;
        ev.obj_id = p->get_id();
        ev.start_time = chrono::system_clock::now() + 3s;
        ev.ev = EVENT_NPC_ATTACK;
        ev.target_id = target->get_id();
        timer_queue.push(ev);
    }
    else {  // 플레이어가 공격을 입힘
        if (target->get_id() == 1180) { // 던전상황
            Player* pl = reinterpret_cast<Player*>(p);
            Player** party_player = dungeons[pl->get_indun_id()]->get_party_palyer();
            for (int i = 0; i < GAIA_ROOM; i++) {
                send_change_hp_packet(reinterpret_cast<Player*>(party_player[i]), target);
            }
        }
        else {
            for (auto& obj : players) {
                if (obj->get_state() != ST_INGAME) continue;
                if (true == is_npc(obj->get_id())) break;   // npc가 아닐때
                if (true == is_near(target->get_id(), obj->get_id())) {      // 근처에 있을때
                    send_change_hp_packet(reinterpret_cast<Player*>(obj), target);
                }
            }
        }

        sc_packet_combat_id packet;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_COMBAT_ID;
        packet.id = target->get_id();

        reinterpret_cast<Player*>(p)->do_send(sizeof(packet), &packet);
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

void player_revive(int client_id)
{
    Player* pl = reinterpret_cast<Player*>(players[client_id]);
    if (pl->join_dungeon_room == true) {
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

    pl->state_lock.lock();
    if (pl->get_state() != ST_DEAD) {
        pl->state_lock.unlock();
        return;
    }
    pl->set_state(ST_INGAME);
    pl->state_lock.unlock();

    // 플레이어 죽은 후 초기화 설정
    pl->set_hp(players[client_id]->get_maxhp());
    pl->set_x(3210);
    pl->set_y(0);
    pl->set_z(940);
    pl->set_exp(pl->get_exp() / 2);
    send_status_change_packet(pl);

    sc_packet_revive packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_REVIVE;
    packet.id = client_id;
    packet.x = pl->get_x();
    packet.y = pl->get_y();
    packet.z = pl->get_z();
    packet.hp = pl->get_hp();
    packet.exp = pl->get_exp();
    pl->do_send(sizeof(packet), &packet);

    // 주변에 있는 얘들에게 시야처리 해주어야함
    pl->vl.lock();
    pl->viewlist.clear();
    pl->vl.unlock();
    for (auto& other : players) {
        if (other->get_id() == client_id) continue;   // 나다
        if (true == is_npc(other->get_id())) break;// 만약 내가 있는 곳에 NPC가 있다면
        other->state_lock.lock();
        if (ST_INGAME != other->get_state()) {
            other->state_lock.unlock();
            continue;
        }
        other->state_lock.unlock();
        if (false == is_near(other->get_id(), client_id)) continue;

        // 여기는 플레이어 처리
        Player* other_player = reinterpret_cast<Player*>(other);
        other_player->vl.lock();
        other_player->viewlist.insert(client_id);
        other_player->vl.unlock();

        send_put_object_packet(other_player, pl);
    }

    // 새로 접속한 플레이어에게 기존 정보를 보내중
    for (auto& other : players) {
        if (other->get_id() == client_id) continue;
        other->state_lock.lock();
        if (ST_INGAME != other->get_state()) {
            other->state_lock.unlock();
            continue;
        }
        other->state_lock.unlock();

        if (false == is_near(other->get_id(), client_id))
            continue;

        // 스크립트와 함께 추가된 부분 
        if (true == is_npc(other->get_id())) {	// 시야에 npc가 있다면 
            if (is_agro_near(client_id, other->get_id())) {
                if (other->get_active() == false) {
                    other->set_active(true);
                    timer_event ev;
                    ev.obj_id = other->get_id();
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_NPC_ATTACK;

                    //  ev.target_id = client_id;
                    ev.target_id = other->get_target_id(); //임시 수정 
                    timer_queue.push(ev);
                    Activate_Npc_Move_Event(other->get_id(), pl->get_id());
                }
            }
        }

        pl->vl.lock();
        pl->viewlist.insert(other->get_id());
        pl->vl.unlock();

        send_put_object_packet(pl, other);
    }
    // 장애물 정보
    pl->ob_vl.lock();
    pl->ob_viewlist.clear();
    pl->ob_vl.unlock();
    for (auto& ob : obstacles) {
        if (RANGE < abs(pl->get_x() - ob.get_x())) continue;
        if (RANGE < abs(pl->get_z() - ob.get_z())) continue;

        pl->ob_vl.lock();
        pl->ob_viewlist.insert(ob.get_id());
        pl->ob_vl.unlock();

        sc_packet_put_object packet;
        packet.id = ob.get_id();
        strcpy_s(packet.name, "");
        packet.object_type = ob.get_tribe();
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_PUT_OBJECT;
        packet.x = ob.get_x();
        packet.y = ob.get_y();
        packet.z = ob.get_z();
        pl->do_send(sizeof(packet), &packet);
    }
}

void return_npc_position(int npc_id)
{
    players[npc_id]->set_target_id(-1); //추가


    if (players[npc_id]->get_active() == true) {
        return;
    }
    unordered_set<int> old_viewlist;
    unordered_set<int> new_viewlist;
    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;
        // if (true == is_npc(obj._id)) continue;   // npc가 아닐때
        if (true == is_npc(obj->get_id())) break;   // npc가 아닐때
        if (true == is_near(npc_id, obj->get_id())) {      // 근처에 있을때
            old_viewlist.insert(obj->get_id());         // npc근처에 플레이어가 있으면 old_viewlist에 플레이어 id를 넣는다
        }
    }

    // 원래 자리로 돌아가자
    players[npc_id]->lua_lock.lock();
    lua_State* L = players[npc_id]->L;
    lua_getglobal(L, "return_my_position");
    int error = lua_pcall(L, 0, 3, 0);
    if (error != 0) {
        players[npc_id]->lua_lock.unlock();
        cout << "LUA_RETURN_MY_POSITION ERROR" << endl;
        return;
    }
    float my_x = lua_tointeger(L, -3);
    float my_y = lua_tointeger(L, -2);
    float my_z = lua_tointeger(L, -1);
    lua_pop(L, 3);
    players[npc_id]->lua_lock.unlock();
    int now_x = players[npc_id]->get_x();
    int now_y = players[npc_id]->get_y();
    int now_z = players[npc_id]->get_z();
    bool my_pos_fail = true;

    pos mv = players[npc_id]->a_star(my_x, my_z, now_x, now_z, obstacles);
    if (abs(mv.first - my_x) <= 10 && abs(mv.second - my_z) <= 10) {
        now_x = my_x;
        now_z = my_z;
        my_pos_fail = false;
    }
    else {
        now_x = mv.first;
        now_z = mv.second;
    }

    float look_x = now_x - players[npc_id]->get_x();
    float look_z = now_z - players[npc_id]->get_z();

    players[npc_id]->set_look(look_x, 0.0f, look_z);

    players[npc_id]->set_x(now_x);
    players[npc_id]->set_z(now_z);


    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;   // in game이 아닐때
        //if (true == is_npc(obj._id)) continue;   // npc가 아닐때 -> ingame중인 플레이어 찾기
        if (true == is_npc(obj->get_id())) break;   // npc가 아닐때 -> ingame중인 플레이어 찾기
        if (true == is_near(npc_id, obj->get_id())) {
            new_viewlist.insert(obj->get_id());
        }
    }

    for (auto pl : new_viewlist) {
        // 새로 시야에 들어온 플레이어
        if (0 == old_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.insert(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_put_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);

        }
        else {
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id], 1);
            send_look_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
    }

    // 시야에 사라진 경우
    for (auto pl : old_viewlist) {
        if (0 == new_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.erase(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_remove_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
    }

    players[npc_id]->state_lock.lock();
    if (players[npc_id]->get_state() != ST_INGAME) {
        players[npc_id]->state_lock.unlock();
        return;
    }
    players[npc_id]->state_lock.unlock();

    if (my_pos_fail) {    // 더 움직여야돼
        timer_event ev;
        ev.obj_id = npc_id;
        ev.start_time = chrono::system_clock::now() + 1s;
        ev.ev = EVENT_NPC_MOVE;
        ev.target_id = -1;
        timer_queue.push(ev);
    }
}

void do_npc_move(int npc_id, int target)
{

    players[npc_id]->lua_lock.lock();
    lua_State* L = players[npc_id]->L;
    lua_getglobal(L, "event_npc_move");
    lua_pushnumber(L, target);
    int error = lua_pcall(L, 1, 1, 0);
    if (error != 0) {
        cout << "LUA_NPC_MOVE ERROR" << endl;
    }
    // bool값도 리턴을 해주자 
    // true면 쫒아간다 
    bool m = lua_toboolean(L, -1);
    lua_pop(L, 1);
    players[npc_id]->lua_lock.unlock();
    if (!m) {
        players[npc_id]->set_active(false);
        return_npc_position(npc_id);
        return;
    }


    unordered_set<int> old_viewlist;
    unordered_set<int> new_viewlist;
    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;
        if (true == is_npc(obj->get_id())) break;   // npc가 아닐때
        if (true == is_near(npc_id, obj->get_id())) {      // 근처에 있을때
            old_viewlist.insert(obj->get_id());         // npc근처에 플레이어가 있으면 old_viewlist에 플레이어 id를 넣는다
        }
    }

    if (old_viewlist.size() == 0) {
        players[npc_id]->set_active(false);
        return_npc_position(npc_id);
        return;
    }

    int x = players[npc_id]->get_x();
    int z = players[npc_id]->get_z();

    int t_x = players[target]->get_x();
    int t_z = players[target]->get_z();

    // 움직일 필요가 없다
    if ((t_x >= x - 8 && t_x <= x + 8) && (t_z >= z - 8 && t_z <= z + 8)) {
        players[npc_id]->state_lock.lock();
        if (players[npc_id]->get_state() != ST_INGAME) {
            players[npc_id]->state_lock.unlock();
            return;
        }
        players[npc_id]->state_lock.unlock();

        timer_event ev;
        ev.obj_id = npc_id;
        ev.start_time = chrono::system_clock::now() + 1s;
        ev.ev = EVENT_NPC_MOVE;
        ev.target_id = target;  //target
        timer_queue.push(ev);
        return;
    }

    // A*알고리즘
    pos mv = players[npc_id]->a_star(t_x, t_z, x, z, obstacles);
    x = mv.first;
    z = mv.second;

    float look_x = x - players[npc_id]->get_x();
    float look_z = z - players[npc_id]->get_z();

    players[npc_id]->set_look(look_x, 0.0f, look_z);
    players[npc_id]->set_x(x);
    players[npc_id]->set_z(z);

    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;   // in game이 아닐때
        //if (true == is_npc(obj._id)) continue;   // npc가 아닐때 -> ingame중인 플레이어 찾기
        if (true == is_npc(obj->get_id())) break;   // npc가 아닐때 -> ingame중인 플레이어 찾기
        if (true == is_near(npc_id, obj->get_id())) {
            new_viewlist.insert(obj->get_id());
        }
    }

    players[npc_id]->vl.lock();
    players[npc_id]->viewlist.clear();
    players[npc_id]->viewlist = new_viewlist;
    players[npc_id]->vl.unlock();

    for (auto pl : new_viewlist) {
        // 새로 시야에 들어온 플레이어
        if (0 == old_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.insert(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_put_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
        else {
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id], 1);
            send_look_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
    }

    // 시야에 사라진 경우
    for (auto pl : old_viewlist) {
        if (0 == new_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.erase(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_remove_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
    }

    players[npc_id]->state_lock.lock();
    if (players[npc_id]->get_state() != ST_INGAME) {
        players[npc_id]->state_lock.unlock();
        return;
    }
    players[npc_id]->state_lock.unlock();

    timer_event ev;
    ev.obj_id = npc_id;
    ev.start_time = chrono::system_clock::now() + 1s;
    ev.ev = EVENT_NPC_MOVE;
    ev.target_id = target;
    timer_queue.push(ev);
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
