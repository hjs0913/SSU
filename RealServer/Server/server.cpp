#include "stdafx.h"
#include "Player.h"
#include "database.h"
#include "send.h"
#include <fstream>
#include <queue>
#include <random>

CRITICAL_SECTION cs;

HANDLE g_h_iocp;
SOCKET g_s_socket;
array <Npc*, MAX_USER + MAX_NPC> players;
array <Obstacle, MAX_OBSTACLE> obstacles;

typedef pair<int, int> pos;

void do_npc_move(int npc_id, int target);
void return_npc_position(int npc_id);

struct timer_event {
    int obj_id;
    chrono::system_clock::time_point start_time;
    EVENT_TYPE ev;
    /*     target_id
    스킬 관련 쿨타임의 경우 : 어떤 스킬인지 넣어줌
    */
    int target_id;      

    constexpr bool operator < (const timer_event& _left) const
    {
        return (start_time > _left.start_time);
    }

};

concurrency::concurrent_priority_queue<timer_event> timer_queue;

void error_display(int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, 0);
    wcout << lpMsgBuf << endl;

    //while (true);
    LocalFree(lpMsgBuf);
}

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
        if ((ob.get_x() - size <= x && x <= ob.get_x() + size) && (ob.get_z() - size <= z && z <= ob.get_z()+size )) {
            cout << "충돌했다" << endl;
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

void Disconnect(int c_id)
{
    Player* pl = reinterpret_cast<Player*>(players[c_id]);
    pl->vl.lock();
    unordered_set <int> my_vl = pl->viewlist;
    pl->vl.unlock();
    for (auto& other : my_vl) {
        Player* target = reinterpret_cast<Player*>(players[other]);
        if (true == is_npc(target->get_id())) continue;   // npc일 경우 보내지 않는다
        if (ST_INGAME != target->get_state() && ST_DEAD!=target->get_state()) continue;
        target->vl.lock();
        if (0 != target->viewlist.count(c_id)) {
            target->viewlist.erase(c_id);
            target->vl.unlock();
            send_remove_object_packet(target, players[c_id]);
        }
        else target->vl.unlock();
    }

    // DB 연결
    /*
    if (players[c_id]->get_state() == ST_INGAME ||
        players[c_id]->get_state() == ST_DEAD) {
        EnterCriticalSection(&cs);
        Save_position(pl);
        LeaveCriticalSection(&cs);
    }
    */

    players[c_id]->state_lock.lock();
    closesocket(reinterpret_cast<Player*>(players[c_id])->_socket);
    players[c_id]->set_state(ST_FREE);
    players[c_id]->state_lock.unlock();
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
    cout << players[target]->get_defence_factor() << endl;
    cout << players[target]->get_magical_defence() << endl;
    cout << "give_damage : " << give_damage << endl;
    cout << "defence_damage : " << defence_damage << endl;
    cout << players[target]->get_defence_factor() *
        players[target]->get_magical_defence() << endl;
    cout << (1 + (players[target]->get_defence_factor() *
        players[target]->get_magical_defence())) << endl;

    cout << p_id << "가 " << damage << "을 " << target << "에게 주었다."
        << target_hp << "남음" << endl;

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


            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 3s;
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
                send_remove_object_packet(other_player, players[target]);
            }
            else other_player->vl.unlock();
        }
    }
    else if (p_id >= NPC_ID_START) {

        //send_status_change_packet(reinterpret_cast<Player*>(players[target]));

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
    cout << players[target]->get_defence_factor() << endl;
    cout << players[target]->get_physical_defence() << endl;
    cout << "give_damage : " << give_damage << endl;
    cout << "defence_damage : " << defence_damage << endl;
    cout << players[target]->get_defence_factor() *
        players[target]->get_physical_defence() << endl;
    cout << (1 + (players[target]->get_defence_factor() *
        players[target]->get_physical_defence())) << endl;

    cout << p_id << "가 " << damage << "을 " << target << "에게 주었다."
        << target_hp << "남음" << endl;

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


            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 3s;
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
                send_remove_object_packet(other_player, players[target]);
            }
            else other_player->vl.unlock();
        }
    }
    else if (p_id >= NPC_ID_START) {

        //send_status_change_packet(reinterpret_cast<Player*>(players[target]));

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

        

        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
       // send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
    }
}

bool superposition = false; //이거를 npc마다 다 주자 

void attack_success(int p_id, int target, float atk_factor)
{



    // 현재 물리 공격에 대해서만 생각한다
    float give_damage = players[p_id]->get_physical_attack() * atk_factor;
    float defence_damage = (players[target]->get_defence_factor() *
        players[target]->get_physical_defence()) / (1 + (players[target]->get_defence_factor() *
            players[target]->get_physical_defence()));
    float damage = give_damage * (1 - defence_damage);
    int target_hp = players[target]->get_hp() - damage;

    cout << players[p_id]->get_name() << "가 " << damage << "의 데미지를 " 
        << players[target]->get_name() << "에게 입혀"
        << target_hp << "의 피가 남음" << endl;


    players[target]->set_hp(target_hp);

    //timer_event ev;
    //ev.obj_id = p_id;
    //ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
    //ev.ev = EVENT_ELEMENT_COOLTIME;;
    //ev.target_id = target;
    //timer_queue.push(ev);

    cout << "공격자  속성" << players[p_id]->get_element() << endl;

    
    switch (players[p_id]->get_element())
    {
     if(superposition)
    case E_WATER:
        if (players[target]->get_element() == E_FULLMETAL || players[target]->get_element() == E_FIRE
            || players[target]->get_element() == E_EARTH) {
            players[target]->set_magical_attack(players[target]->get_magical_attack() / 10 * 9);
            superposition = true;//npc안에 별도로 만들자 
        }
        cout << "타켓의 마공:" << players[target]->get_magical_attack() << endl;
        break;
    case E_FULLMETAL:
        if (players[target]->get_element() == E_ICE || players[target]->get_element() == E_TREE
            || players[target]->get_element() == E_WIND) {
            reinterpret_cast<Player*>(players[p_id])->set_physical_defence(reinterpret_cast<Player*>(players[p_id])->get_physical_defence() + reinterpret_cast<Player*>(players[p_id])->get_physical_defence() / 10);
            superposition = true;
        }
        break;
    case E_WIND:
        if (players[target]->get_element() == E_WATER || players[target]->get_element() == E_EARTH
            || players[target]->get_element() == E_FIRE) {
           //공속 시전속도 상승 , 쿨타임 감소 
            superposition = true;//npc안에 별도로 만들자 
        }
        break;
    case E_FIRE:
        if (players[target]->get_element() == E_ICE || players[target]->get_element() == E_TREE
            || players[target]->get_element() == E_FULLMETAL) {
        //10초 공격력 10프로의 화상 피해 
            superposition = true;//npc안에 별도로 만들자 
        }
        break;
    case E_TREE:
        if (players[target]->get_element() == E_EARTH || players[target]->get_element() == E_WATER
            || players[target]->get_element() == E_WIND) {
            players[target]->set_physical_attack(players[target]->get_physical_attack() / 10 * 9);
            superposition = true;//npc안에 별도로 만들자 
        }
        break;
    case E_EARTH:
        if (players[target]->get_element() == E_ICE || players[target]->get_element() == E_FULLMETAL
            || players[target]->get_element() == E_FIRE) {
            reinterpret_cast<Player*>(players[p_id])->set_magical_defence(reinterpret_cast<Player*>(players[p_id])->get_magical_defence() + reinterpret_cast<Player*>(players[p_id])->get_magical_defence() / 10);
            superposition = true;//npc안에 별도로 만들자 
        }
        break;
    case E_ICE:
        if (players[target]->get_element() == E_TREE || players[target]->get_element() == E_WATER
            || players[target]->get_element() == E_WIND) {
         //동결 and  10초동안 공속, 시전속도, 이동속도 10프로감소 
            superposition = true;//npc안에 별도로 만들자 
        }
        break;
    default:
        break;
    }

    ;
  //  EXP_OVER* exp_over = new EXP_OVER;
   // exp_over->_comp_op = OP_ELEMENT_COOLTIME;
  //  exp_over->_target = target;
  //  PostQueuedCompletionStatus(g_h_iocp, p_id, target, &exp_over->_wsa_over);


    if (target_hp <= 0) {
        players[target]->state_lock.lock();
        if(players[target]->get_state()!=ST_INGAME){
            players[target]->state_lock.unlock();
            return;
        }
        players[target]->set_state(ST_DEAD);
        players[target]->state_lock.unlock();
        if (target < NPC_ID_START) {    // 죽은게 플레이어이다
            players[p_id]->set_active(false);
            // 죽은것이 플레이어라면 죽었다는 패킷을 보내준다
            sc_packet_dead packet;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_DEAD;
            packet.id = target;
            packet.attacker_id = p_id;
            reinterpret_cast<Player*>(players[target]) ->do_send(sizeof(packet), &packet);
            
            // 3초후 부활하며 부활과 동시에 위치 좌표를 수정해준다
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 3s;
            ev.ev = EVENT_PLAYER_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);
        }
        else {  // NPC라면 30초 후에 부활할 수 있도록 하자
            players[target]->set_active(false);
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 30s;
            ev.ev = EVENT_NPC_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);

            // 플레이어에게 경험치 제공, 그리고 바뀐 경험치와 레벨을 보내주자
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
                    set_exp(reinterpret_cast<Player*>(players[p_id])->get_exp()+ get_exp - max_exp);
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
        // 죽은 target 주위의 플레이어에게 사라지게 해주자
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
                send_remove_object_packet(other_player, players[target]);
            }
            else other_player->vl.unlock();
        }
    }
    else if(p_id >= NPC_ID_START){
        // 플레이어가 공격을 당한 것이므로 hp정보가 바뀌었으므로 그것을 보내주자
        // send_status_change_packet(reinterpret_cast<Player*>(players[target]));
        
        // 플레이어의 ViewList에 있는 플레이어들에게 보내주자
        send_change_hp_packet(reinterpret_cast<Player*>(players[target]), players[target]);
        reinterpret_cast<Player*>(players[target])->vl.lock();
        for (auto id : reinterpret_cast<Player*>(players[target])->viewlist) {
            if (true == is_npc(id)) continue;
            send_change_hp_packet(reinterpret_cast<Player*>(players[id]), players[target]);
        }
        reinterpret_cast<Player*>(players[target])->vl.unlock();


        char mess[MAX_CHAT_SIZE];
        //send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

        // hp가 깎이였으므로 hp자동회복을 해주도록 하자
        if (reinterpret_cast<Player*>(players[target])->_auto_hp == false) {
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 5s;
            ev.ev = EVENT_AUTO_PLAYER_HP;
            ev.target_id = 0;
            timer_queue.push(ev);
            reinterpret_cast<Player*>(players[target])->_auto_hp = true;
        }

        // npc공격이면 타이머 큐에 다시 넣어주자
        timer_event ev;
        ev.obj_id = p_id;
        ev.start_time = chrono::system_clock::now() + 3s;
        ev.ev = EVENT_NPC_ATTACK;
        ev.target_id = target;
        timer_queue.push(ev);
    }
    else {  // 플레이어가 공격을 입힘
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
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %f",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        //send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
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
void process_packet(int client_id, unsigned char* p)
{
   
    unsigned char packet_type = p[1];
    Player* pl = reinterpret_cast<Player*>(players[client_id]);
    switch (packet_type) {
    case CS_PACKET_LOGIN: {
        cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);

        // pl->set_name(packet->name);

        // DB 연결
        /*
        EnterCriticalSection(&cs);
        if (!(Search_Id(pl, packet->name))) {
            send_login_fail_packet(pl, 0);   // 아이디 없음
            Disconnect(client_id);
            LeaveCriticalSection(&cs);
            return;
        }
        LeaveCriticalSection(&cs);
        */

        // 중복 아이디 검사
        /*
        for (auto* p : players) {
            if (p->get_tribe() != HUMAN) break;
            if (p->get_state() == ST_FREE) continue;
            if (p->get_id() == client_id) continue;
            if (reinterpret_cast<Player*>(p)->get_login_id() == pl->get_login_id()) {
                send_login_fail_packet(pl, 1);   // 중복 로그인
                Disconnect(client_id);
                return;
            }
        }
        */
        // 원래는 DB에서 받아와야 하는 정보를 기본 정보로 대체
        pl->set_x(2100);
        pl->set_y(0);
        pl->set_z(1940);
        pl->set_job(J_TANKER);
        pl->set_lv(25);
        pl->set_element(E_WATER);
        pl->set_name("정의범");
        switch (pl->get_job()) {
        case J_DILLER: {
            int lv = pl->get_lv();
            pl->set_maxhp(20 * lv * lv + 80 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(10 * lv * lv + 50 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.3 * lv * lv + 10 * lv);
            pl->set_magical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_physical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
            break;
        }
        case J_TANKER: {
            int lv = pl->get_lv();
            pl->set_maxhp(22 * lv * lv + 80 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(8.5 * lv * lv + 50 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.25 * lv * lv + 10 * lv);
            pl->set_magical_attack(0.08 * lv * lv + 5 * lv);
            pl->set_physical_defence(0.27 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.2 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
            pl->set_element(E_WATER);
            break;
        }
        case J_SUPPORTER: {  //값은 바꿔야함 
            int lv = pl->get_lv();
            pl->set_maxhp(22 * lv * lv + 80 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(8.5 * lv * lv + 50 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.25 * lv * lv + 10 * lv);
            pl->set_magical_attack(0.08 * lv * lv + 5 * lv);
            pl->set_physical_defence(0.27 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.2 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
            break;
        }

        default: {
            cout << "없는 직업" << endl;
            break;
        }
        }
        // -- DB 대체 끝 --

        // Hp회복
        if (pl->get_hp() < pl->get_maxhp()) {
            // hp가 깎이였으므로 hp자동회복을 해주도록 하자
            if (reinterpret_cast<Player*>(players[client_id])->_auto_hp == false) {
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;
                ev.ev = EVENT_AUTO_PLAYER_HP;
                ev.target_id = 0;
                timer_queue.push(ev);
                reinterpret_cast<Player*>(players[client_id])->_auto_hp = true;
            }
        }

        send_login_ok_packet(pl);
        pl->state_lock.lock();
        pl->set_state(ST_INGAME);
        pl->state_lock.unlock();

        // 새로 접속한 정보를 다른이에게 보내줌
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

            /*sc_packet_put_object packet;
            packet.id = client_id;
            strcpy_s(packet.name, pl->get_name());
            packet.object_type = pl->get_tribe();
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_PUT_OBJECT;
            packet.x = pl->get_x();
            packet.y = pl->get_y();
            packet.z = pl->get_z();
            other_player->do_send(sizeof(packet), &packet);*/
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
                        ev.target_id = client_id;
                        timer_queue.push(ev);
                        Activate_Npc_Move_Event(other->get_id(), pl->get_id());
                    }
                }
            }

            pl->vl.lock();
            pl->viewlist.clear();
            pl->viewlist.insert(other->get_id());
            pl->vl.unlock();

            send_put_object_packet(pl, other);
            /*sc_packet_put_object packet;
            packet.id = other->get_id();
            strcpy_s(packet.name, other->get_name());
            packet.object_type = other->get_tribe();
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_PUT_OBJECT;
            packet.x = other->get_x();
            packet.y = other->get_y();
            packet.z = other->get_z();
            pl->do_send(sizeof(packet), &packet);*/
        }
        // 장애물 정보
        for (auto& ob : obstacles) {
            if (RANGE < abs(pl->get_x() - ob.get_x())) continue;
            if (RANGE < abs(pl->get_z() - ob.get_z())) continue;

            pl->ob_vl.lock();
            pl->ob_viewlist.clear();
            pl->ob_viewlist.insert(ob.get_id());
            pl->ob_vl.unlock();

            //send_put_object_packet(pl, ob);
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
        break;
    }
    case CS_PACKET_MOVE: {
        cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
        // pl.last_move_time = packet->move_time;
        pl->last_move_time = packet->move_time;
        float x = packet->x;
        float y = packet->y;
        float z = packet->z;       

        // 유효성 검사
        if (check_move_alright(x, z, false) == false) {
            // 올바르지 않을경우 위치를 수정을 해주어야 한다
            // 클라쪽 수정 필요
            send_move_packet(pl, pl);
            break;
        }

        pl->set_x(x);
        pl->set_y(y);
        pl->set_z(z);
        unordered_set <int> nearlist;
        for (auto& other : players) {
            // if (other._id == client_id) continue;
            if (false == is_near(client_id, other->get_id()))
                continue;
            if (ST_INGAME != other->get_state())
                continue;
            //스크립트 추가
            if (true == is_npc(other->get_id())) {
                if (is_agro_near(client_id, other->get_id())) {
                    if (other->get_active() == false) {
                        other->set_active(true);
                        timer_event ev;
                        ev.obj_id = other->get_id();
                        ev.start_time = chrono::system_clock::now() + 1s;
                        ev.ev = EVENT_NPC_ATTACK;
                        ev.target_id = client_id;
                        timer_queue.push(ev);
                        Activate_Npc_Move_Event(other->get_id(), pl->get_id());
                    }
                }
            }
            nearlist.insert(other->get_id());
        }
        nearlist.erase(client_id);  // 내 아이디는 무조건 들어가니 그것을 지워주자

        send_move_packet(pl, pl); // 내 자신의 움직임을 먼저 보내주자

        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

        // 새로시야에 들어온 플레이어 처리
        for (auto other : nearlist) {
            if (0 == my_vl.count(other)) {   // 원래 없던 플레이어/npc
                pl->vl.lock();
                pl->viewlist.insert(other);
                pl->vl.unlock();
                send_put_object_packet(pl, players[other]);

                // 스크립트 추가
                if (true == is_npc(other)) break;

                // 여기는 플레이어 처리이다.
                Player* other_player = reinterpret_cast<Player*>(players[other]);
                other_player->vl.lock();
                if (0 == other_player->viewlist.count(pl->get_id())) {
                    other_player->viewlist.insert(pl->get_id());
                    other_player->vl.unlock();
                    send_put_object_packet(other_player, pl);
                    
                }
                else {
                    other_player->vl.unlock();
                    send_move_packet(other_player, pl);
                }
            }
            // 계속 시야에 존재하는 플레이어 처리
            else {
                if (true == is_npc(other)) continue;   // 원래 있던 npc는 npc_move에서 처리

                Player* other_player = reinterpret_cast<Player*>(players[other]);
                other_player->vl.lock();
                if (0 == other_player->viewlist.count(pl->get_id())) {
                    other_player->viewlist.insert(pl->get_id());
                    other_player->vl.unlock();
                    send_put_object_packet(other_player, pl);
                }
                else {
                    other_player->vl.unlock();
                    send_move_packet(other_player, pl);
                }
            }
        }
        // 시야에서 사라진 플레이어 처리
        for (auto other : my_vl) {
            if (0 == nearlist.count(other)) {
                pl->vl.lock();
                pl->viewlist.erase(other);
                pl->vl.unlock();
                send_remove_object_packet(pl, players[other]);

                if (true == is_npc(other)) continue;
                Player* other_player = reinterpret_cast<Player*>(players[other]);
                other_player->vl.lock();
                if (0 != other_player->viewlist.count(pl->get_id())) {
                    other_player->viewlist.erase(pl->get_id());
                    other_player->vl.unlock();
                    send_remove_object_packet(other_player, pl);
                }
                else other_player->vl.unlock();
            }
        }
        // 새로 생긴 장애물이 존재 가능
        // 장애물 정보

        for (auto& ob : obstacles) {
            if ((RANGE < abs(pl->get_x() - ob.get_x())) &&
                (RANGE < abs(pl->get_z() - ob.get_z()))) {
                // 범위 벗어난거임(존재하던게 있으면 없애자)
                pl->ob_vl.lock();
                if (pl->ob_viewlist.count(ob.get_id()) != 0) {
                    pl->ob_viewlist.erase(ob.get_id());
                    pl->ob_vl.unlock();
                    sc_packet_remove_object packet;
                    packet.size = sizeof(packet);
                    packet.type = SC_PACKET_REMOVE_OBJECT;
                    packet.id = ob.get_id();
                    packet.object_type = ob.get_tribe();
                    pl->do_send(sizeof(packet), &packet);
                    continue;
                }
                pl->ob_vl.unlock();
                continue;
            }
            // 이미 존재하는가
            pl->ob_vl.lock();
            if (pl->ob_viewlist.count(ob.get_id()) != 0) {
                pl->ob_vl.unlock();
                continue;
            }
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
            //pl->do_send(sizeof(packet), &packet);
            pl->do_send(sizeof(packet), &packet);
                
        }
        break;
    }
    case CS_PACKET_ATTACK: {
        // cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(p);
        // 플레이어가 공격하고 반경 1칸 이내에 몬스터가 있다면 전투


        if (pl->get_attack_active()) break;
        pl->set_attack_active(true);

        timer_event ev;
        ev.obj_id = client_id;
        ev.start_time = chrono::system_clock::now() + 1s;
        ev.ev = EVENT_PLAYER_ATTACK;
        ev.target_id = client_id;
        timer_queue.push(ev);
        
        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
            players[i]->state_lock.lock();
            if (players[i]->get_state() != ST_INGAME) {
                players[i]->state_lock.unlock();
                continue;
            }
            players[i]->state_lock.unlock();
            if (players[i]->get_x() >= pl->get_x() -10 && players[i]->get_x() <= pl->get_x() + 10) {
                if (players[i]->get_z() >= pl->get_z() - 10 && players[i]->get_z() <= pl->get_z() + 10) {
                    attack_success(client_id, players[i]->get_id(), pl->get_basic_attack_factor());    // 데미지 계산
                    // 몬스터의 자동공격을 넣어주자
                    players[i]->set_target_id(pl->get_id());
                    if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                        players[i]->set_active(true);
                        ev.obj_id = i;
                        ev.start_time = chrono::system_clock::now() + 1s;
                        ev.ev = EVENT_NPC_ATTACK;
                        ev.target_id = players[i]->get_target_id();
                        timer_queue.push(ev);
                        // 몬스터의 이동도 넣어주자
                        Activate_Npc_Move_Event(i, players[i]->get_target_id());
                    }
                }
            }
        }
        break;
    }
    case CS_PACKET_CHAT: {
        cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(p);
        char c_temp[MAX_CHAT_SIZE];
        sprintf_s(c_temp, MAX_CHAT_SIZE, "%s : %s", pl->get_name(), packet->message);
        for (auto& s_pl : players) {
            s_pl->state_lock.lock();
            if (s_pl->get_state() != ST_INGAME) {
                s_pl->state_lock.unlock();
                continue;
            }
            s_pl->state_lock.unlock();
            if (s_pl->get_tribe() == MONSTER) break;
            
            send_chat_packet(reinterpret_cast<Player*>(s_pl), client_id, c_temp);
            
        }
        break;
    }
    case CS_PACKET_TELEPORT: {
        pl->set_x(rand() % WORLD_WIDTH);
        pl->set_y(rand() % WORLD_HEIGHT);
        sc_packet_move packet;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_MOVE;
        packet.id = pl->get_id();
        packet.x = pl->get_x();
        packet.y = pl->get_y();
        packet.z = pl->get_z();
        packet.move_time = pl->last_move_time;
        break;
    }
    case CS_PACKET_SKILL: {
        cs_packet_skill* packet = reinterpret_cast<cs_packet_skill*>(p);
        if (pl->get_skill_active(packet->skill_type) == true) return;
        pl->set_skill_active(packet->skill_type, true);     //일반공격 계수는 5
        cout << "직업은 " << pl->get_job() << endl;
        switch (pl->get_job())
        {
        case J_DILLER:
            switch (packet->skill_type)
            {
            case 0:    // 물리 공격 스킬 
                switch (packet->skill_num)
                {
                case 0:  //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 0;
                    timer_queue.push(ev);

                    cout << "최후의 일격 !!!" << endl;
                    pl->set_mp(pl->get_mp() - 1000);

                    for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                        players[i]->state_lock.lock();
                        if (players[i]->get_state() != ST_INGAME) {
                            players[i]->state_lock.unlock();
                            continue;
                        }
                        players[i]->state_lock.unlock();

                        if ((players[i]->get_x() >= pl->get_x() - 10 && players[i]->get_x() <= pl->get_x() + 10) && (players[i]->get_z() >= pl->get_z() - 10 && players[i]->get_z() <= pl->get_z() + 10)) {
                            pl->set_skill_factor(packet->skill_type, packet->skill_num);
                            physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                            players[i]->set_target_id(pl->get_id());
                            send_status_change_packet(pl);
                            if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                players[i]->set_active(true);
                                timer_event ev;
                                ev.obj_id = i;
                                ev.start_time = chrono::system_clock::now() + 1s;
                                ev.ev = EVENT_NPC_ATTACK;
                                ev.target_id = players[i]->get_target_id();
                                timer_queue.push(ev);

                                Activate_Npc_Move_Event(i, pl->get_id());
                            }
                        }
                    }

                    break;
                }
                break;
            case 1:  //마법 공격 스킬  삼각형 범위?
                switch (packet->skill_num)
                {
                case 0:  //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 1;
                    timer_queue.push(ev);

                    cout << "look : " << pl->get_look_x() << ", " << pl->get_look_z() << endl;
                    cout << "right : " << pl->get_right_x() << ", " << pl->get_right_z() << endl;

                    Coord a = { pl->get_x(), pl->get_z() };    //플레이어 기준 전방 삼각형 범위 
                    Coord b = { pl->get_x() - pl->get_right_x() * 40 + pl->get_look_x() * 100,
                        pl->get_z() - pl->get_right_z() * 40 + pl->get_look_z() * 100 };  // 왼쪽 위
                    Coord c = { pl->get_x() + pl->get_right_x() * 40 + pl->get_look_x() * 100,
                        pl->get_z() + pl->get_right_z() * 40 + pl->get_look_z() * 100 };  // 오른쪽 위
                    cout << " 원래 좌표 : " << a.x << ", " << a.z << endl;
                    cout << " 왼쪽 좌표 : " << b.x << ", " << b.z << endl;
                    cout << " 오른쪽 좌표 : " << c.x << ", " << c.z << endl;

                    cout << "광야 일격 !!!" << endl;
                    pl->set_mp(pl->get_mp() - 1000);
                    for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                        players[i]->state_lock.lock();
                        if (players[i]->get_state() != ST_INGAME) {
                            players[i]->state_lock.unlock();
                            continue;
                        }
                        players[i]->state_lock.unlock();

                        Coord n = { players[i]->get_x(), players[i]->get_z() };
                        float px = players[i]->get_x();
                        float pz = players[i]->get_z();

                        if (isInsideTriangle(a, b, c, n)) {
                            cout << "여기 들어오는가 1 : " << i << endl;
                            cout << "맞은놈 좌표 : " << n.x << ", " << n.z << endl;
                            pl->set_skill_factor(packet->skill_type, packet->skill_num);
                            cout << pl->get_skill_factor(packet->skill_type, packet->skill_num) << endl;
                            magical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                            players[i]->set_target_id(pl->get_id());
                            send_status_change_packet(pl);
                            if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                players[i]->set_active(true);
                                timer_event ev;
                                ev.obj_id = i;
                                ev.start_time = chrono::system_clock::now() + 1s;
                                ev.ev = EVENT_NPC_ATTACK;
                                ev.target_id = players[i]->get_target_id();
                                timer_queue.push(ev);
                                Activate_Npc_Move_Event(i, pl->get_id());
                            }
                        }
                    }
                    break;
                }

                break;
            case 2:  //버프   //공격력 증가로 변경 
                switch (packet->skill_num)
                {
                case 0:
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);



                    cout << pl->get_physical_attack() << endl;
                    cout << pl->get_magical_attack() << endl;
                    cout << "아레스의 가호 !!!" << endl;
                    cout << pl->get_mp() << endl;
                    pl->set_mp(pl->get_mp() - 1000);

                    pl->set_physical_attack(0.6 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv()); //일단 두배 
                    pl->set_magical_attack(0.2 * pl->get_lv() * pl->get_lv() + 5 * pl->get_lv());
                    send_status_change_packet(pl);
                    cout << pl->get_physical_attack() << endl;
                    cout << pl->get_magical_attack() << endl;

                   

                    break;
                }
                break;
            }
            break;
          
            case J_TANKER:
                switch (packet->skill_type)
                {
                case 0:    // 물리 공격 스킬 // 방패 
                    switch (packet->skill_num)
                    {
                    case 0:  //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                        timer_event ev;
                        ev.obj_id = client_id;
                        ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                        ev.ev = EVENT_SKILL_COOLTIME;
                        ev.target_id = 0;
                        timer_queue.push(ev);

                        cout << "밀어내기 !!!" << endl;
                        pl->set_mp(pl->get_mp() - 1000);

                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                pl->set_skill_factor(packet->skill_type, packet->skill_num);
                                physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));

                                players[i]->set_pos(players[i]->get_x() + pl->get_look_x()*40, players[i]->get_z() + pl->get_look_z() * 40);
                                send_move_packet(pl, players[i]);  //나중에 수정필요 
                                send_status_change_packet(pl);
                                players[i]->set_target_id(pl->get_id());
                               if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 3s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    timer_queue.push(ev);

                                    Activate_Npc_Move_Event(i, pl->get_id());
                                }
                            }
                        }

                        break;
                    }
                    break;
                case 1:  //마법 공격 스킬:  어그로   다른 플레이어가 공격 도중 쓰면 대상이 나로 안바뀐다 수정 필요 
                    switch (packet->skill_num)
                    {
                    case 0:   //어그로
                        timer_event ev;
                        ev.obj_id = client_id;
                        ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                        ev.ev = EVENT_SKILL_COOLTIME;
                        ev.target_id = 1;
                        timer_queue.push(ev);

                        cout << "look : " << pl->get_look_x() << ", " << pl->get_look_z() << endl;
                        cout << "right : " << pl->get_right_x() << ", " << pl->get_right_z() << endl;

                        cout << "어그로 끌기!!!" << endl;
                        pl->set_mp(pl->get_mp() - 1000);
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            if ((players[i]->get_x() >= pl->get_x() - 40 && players[i]->get_x() <= pl->get_x() + 40) && (players[i]->get_z() >= pl->get_z() - 40 && players[i]->get_z() <= pl->get_z() + 40)) {
                                pl->set_skill_factor(packet->skill_type, packet->skill_num);
                              //  physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                                players[i]->set_target_id(pl->get_id());
                                send_status_change_packet(pl);
                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                   
                                    timer_event ev;
                                     ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                     ev.ev = EVENT_NPC_MOVE;
                                 
                                    ev.target_id = players[i]->get_id();
                                   //  ev.target_id = client_id;
                                     timer_queue.push(ev);

                                    Activate_Npc_Move_Event(i,  players[i]->get_target_id());
                                }
                            }


                        }
                        break;
                    }

                    break;
                case 2:  //버프  방어력 증가 
                    switch (packet->skill_num)
                    {
                    case 0:
                        timer_event ev;
                        ev.obj_id = client_id;
                        ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
                        ev.ev = EVENT_SKILL_COOLTIME;
                        ev.target_id = 2;
                        timer_queue.push(ev);

                        cout << pl->get_physical_defence() << endl;
                        cout << pl->get_magical_defence() << endl;
                        cout << "아테네의 가호 !!!" << endl;
                        cout << pl->get_mp() << endl;
                        pl->set_mp(pl->get_mp() - 1000);

                        pl->set_physical_defence(0.54 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv()); //일단 두배 
                        pl->set_magical_defence(0.4 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv());
                        send_status_change_packet(pl);
                        cout << pl->get_physical_defence() << endl;
                        cout << pl->get_magical_defence() << endl;

                        break;
                    }
                    break;
                }
                break;
                case J_SUPPORTER://서포터 
                    switch (packet->skill_type)
                    {
                    case 2: //버프
                        switch (packet->skill_num)
                        {
                        case 0:  // 사각형 내부 범위 플레이어 hp 회복  

                            timer_event ev;
                            ev.obj_id = client_id;
                            ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                            ev.ev = EVENT_SKILL_COOLTIME;
                            ev.target_id = 2;
                            timer_queue.push(ev);

                            cout << "천사의 치유!!!" << endl;
                            pl->set_mp(pl->get_mp() - 1000);
                            send_status_change_packet(pl);

                            for (int i = 0; i <= MAX_USER; ++i) {
                                players[i]->state_lock.lock();
                                if (players[i]->get_state() != ST_INGAME) {
                                    players[i]->state_lock.unlock();
                                    continue;
                                }
                                players[i]->state_lock.unlock();
                                if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                    cout << "아이디 " <<i<<"의 이전 hp" << players[i]->get_hp() << endl;
                                    players[i]->set_hp(players[i]->get_hp() + players[i]->get_maxhp() / 10);
                                    send_status_change_packet(reinterpret_cast<Player*>(players[i]));
                                    
                                    cout << "체력 회복" << endl;
                                    cout << "아이디 " << i << "의 이후 hp" << players[i]->get_hp() << endl;
                                 
                                }
                            }
                            break;
                       // case 1: // 전방으로 보호막을 날려 닿는 사람만 보호막 생성 

                        }
                        break;
                    }
                    break;



        }
        break;
    }
    case CS_PACKET_LOOK: {
        cs_packet_look* packet = reinterpret_cast<cs_packet_look*>(p);
        pl->set_look(packet->x, packet->y, packet->z);
        pl->set_right(packet->right_x, packet->right_y, packet->right_z);

        // 근처에 있는 모든 플레이어에게 방향이 바뀌었다는것을 보내준다
        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

        for (auto i : my_vl) {
            // Npc
            if (is_npc(i) == true) continue;

            // Player
            send_look_packet(reinterpret_cast<Player*>(players[i]),
                pl);
        }
        break;
    }
    case CS_PACKET_CHANGE_JOB: {
        cs_packet_change_job* packet = reinterpret_cast<cs_packet_change_job*>(p);
        pl->set_job(packet->job);
        cout << "내 직업은" << packet->job << "번 입니다." << endl;
        send_status_change_packet(pl);
        break;
    }
    case CS_PACKET_CHANGE_ELEMENT: {
        cs_packet_change_element* packet = reinterpret_cast<cs_packet_change_element*>(p);
        pl->set_element(packet->element);
        cout << "내 속성은" << packet->element << "번 입니다." << endl;
        send_status_change_packet(pl);
        break;
    }
    }
}

void player_revive(int client_id)
{
    Player* pl = reinterpret_cast<Player*>(players[client_id]);
    pl->state_lock.lock();
    if (pl->get_state() != ST_DEAD) {
        pl->state_lock.unlock();
        return;
    }
    pl->set_state(ST_INGAME);
    pl->state_lock.unlock();
    // 플레이어 죽은 후 초기화 설정
    pl->set_hp(players[client_id]->get_maxhp());
    pl->set_x(0);
    pl->set_y(0);
    pl->set_z(0);
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
        /*sc_packet_put_object packet;
        packet.id = client_id;
        strcpy_s(packet.name, pl->get_name());
        packet.object_type = 0;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_PUT_OBJECT;
        packet.x = pl->get_x();
        packet.y = pl->get_y();
        packet.z = pl->get_z();
        other_player->do_send(sizeof(packet), &packet);*/
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
       /* sc_packet_put_object packet;
        packet.id = other->get_id();
        strcpy_s(packet.name, other->get_name());
        packet.object_type = 0;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_PUT_OBJECT;
        packet.x = other->get_x();
        packet.y = other->get_y();
        packet.z = other->get_z();
        pl->do_send(sizeof(packet), &packet);*/
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

void worker()
{
    for (;;) {
        DWORD num_byte;
        LONG64 iocp_key;
        WSAOVERLAPPED* p_over;
        BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);
        int client_id = static_cast<int>(iocp_key);
        EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);
        if (FALSE == ret) {
            int err_no = WSAGetLastError();
            error_display(err_no);
            Disconnect(client_id);
            if (exp_over->_comp_op == OP_SEND)
                delete exp_over;
            continue;
        }

        switch (exp_over->_comp_op) {
        case OP_RECV: {
            if (num_byte == 0) {
                Disconnect(client_id);
                continue;
            }

            Player* pl = reinterpret_cast<Player*>(players[client_id]);
            int remain_data = num_byte + pl->_prev_size;
            unsigned char* packet_start = exp_over->_net_buf;
            int packet_size = packet_start[0];

            while (packet_size <= remain_data) {
                process_packet(client_id, packet_start);
                remain_data -= packet_size;
                packet_start += packet_size;
                if (remain_data > 0) packet_size = packet_start[0];
                else break;
            }

            if (0 < remain_data) {
                pl->_prev_size = remain_data;
                memcpy(&exp_over->_net_buf, packet_start, remain_data);
            }
            if (remain_data == 0) pl->_prev_size = 0;
            if (pl->get_state() == ST_FREE) continue;
            pl->do_recv();
            break;
        }
        case OP_SEND: {
            if (num_byte != exp_over->_wsa_buf.len) {
                Disconnect(client_id);
            }
            delete exp_over;
            break;
        }
        case OP_ACCEPT: {
            cout << "Accept Completed.\n";
            SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));
            int new_id = get_new_id();
            if (-1 == new_id) {
                cout << "Maxmum user overflow. Accept aborted.\n";
            }
            else {
                //players[new_id] = new Player(new_id);
                Player* pl = reinterpret_cast<Player*>(players[new_id]);
                pl->set_id(new_id);
                pl->_prev_size = 0;
                pl->_recv_over._comp_op = OP_RECV;
                pl->_recv_over._wsa_buf.buf = reinterpret_cast<char*>(pl->_recv_over._net_buf);
                pl->_recv_over._wsa_buf.len = sizeof(pl->_recv_over._net_buf);
                pl->set_tribe(HUMAN);
                ZeroMemory(&pl->_recv_over._wsa_over, sizeof(pl->_recv_over._wsa_over));
                pl->_socket = c_socket;

                CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, new_id, 0);
                pl->do_recv();
            }

            ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
            c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            *(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
            AcceptEx(g_s_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
                sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);
            break;
        }
        case OP_NPC_MOVE: {
            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME)) {
                players[client_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                delete exp_over;
                break;
            }
            players[client_id]->state_lock.unlock();
            // 제자리로 돌아가는 것인가
            if (exp_over->_target == -1) {
                return_npc_position(client_id);
                delete exp_over;
                break;
            }

            int target_id = exp_over->_target;
            players[target_id]->state_lock.lock();

            //쫒아가던 타겟이 살아있는가
            if (players[target_id]->get_state() != ST_INGAME) {
                players[target_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                return_npc_position(client_id);
                delete exp_over;
                break;
            }
            players[target_id]->state_lock.unlock();

            if(players[client_id]->get_target_id() != -1)
               do_npc_move(client_id, exp_over->_target);

            /*
            players[client_id]->lua_lock.lock();
            lua_State* L = players[client_id]->L;
            lua_getglobal(L, "event_npc_move");
            lua_pushnumber(L, exp_over->_target);
            int error = lua_pcall(L, 1, 1, 0);
            if (error != 0) {
                cout << "LUA_NPC_MOVE ERROR" << endl;
            }
            // bool값도 리턴을 해주자 
            // true면 쫒아간다 
            bool m = lua_toboolean(L, -1);
            lua_pop(L, 1);
            players[client_id]->lua_lock.unlock();
            if (m) {
                do_npc_move(client_id, exp_over->_target);
            }
            else {
                // 원래 자리로 돌아가자
                players[client_id]->set_active(false);
                return_npc_position(client_id);
            }
            */
            delete exp_over;
            
            break;
        }
        case OP_NPC_ATTACK: {
            cout << "???" << endl;
            // 죽은 상태나 공격하는 상태인지 아닌지 확인
            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME) || (false == players[client_id]->get_active())) {
                players[client_id]->state_lock.unlock();
                delete exp_over; EVENT_PLAYER_ATTACK;
                break;
            }
            players[client_id]->state_lock.unlock();

            players[client_id]->lua_lock.lock();
            lua_State* L = players[client_id]->L;
            lua_getglobal(L, "attack_range");
            lua_pushnumber(L, exp_over->_target);
            int error = lua_pcall(L, 1, 1, 0);
            if (error != 0) {
                cout << "LUA ATTACK RANGE ERROR" << endl;
                cout << lua_tostring(L, -1) << endl;
            }
            bool m = false;
            m = lua_toboolean(L, -1);
            lua_pop(L, 1);
            if (m) {
                // 공격처리
                cout << "???2" << endl;
                attack_success(client_id, exp_over->_target, players[client_id]->get_basic_attack_factor());
            }
            else {
                if (players[client_id]->get_active()) {
                    // 공격은 실패했지만 계속(그렇지만 1초후) 공격시도
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_NPC_ATTACK;
                    ev.target_id = exp_over->_target;
                    timer_queue.push(ev);
                }
            }
            players[client_id]->lua_lock.unlock();
            delete exp_over;
            break;
        }
                        
        case OP_AUTO_PLAYER_HP: {
            Player* pl = reinterpret_cast<Player*>(players[client_id]);
            pl->state_lock.lock();
            if (pl->get_state() != ST_INGAME) {
                pl->state_lock.unlock();
                break;
            }
            pl->state_lock.unlock();
            pl->set_hp(pl->get_hp() + (pl->get_maxhp()*0.1));
            if (pl->get_hp() >= pl->get_maxhp()) {
                pl->set_hp(pl->get_maxhp());
                reinterpret_cast<Player*>(players[client_id])->_auto_hp = false;
            }
            else {
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;
                ev.ev = EVENT_AUTO_PLAYER_HP;
                ev.target_id = 0;
                timer_queue.push(ev);
            }
            send_change_hp_packet(pl, pl);
            //send_status_change_packet(pl);
            break;
        }
        case OP_PLAYER_REVIVE: {
            player_revive(client_id);
            delete exp_over;
            break;
        }
        case OP_NPC_REVIVE: {
            // 상태 바꿔주고
            players[client_id]->state_lock.lock();
            players[client_id]->set_state(ST_INGAME);
            players[client_id]->state_lock.unlock();
            // NPC의 정보 가져오기
            players[client_id]->lua_lock.lock();
            lua_State* L = players[client_id]->L;
            lua_getglobal(L, "monster_revive");
            int error = lua_pcall(L, 0, 4, 0);
            if (error != 0) {
                cout << "초기화 오류" << endl;
            }

            players[client_id]->set_x(lua_tonumber(L, -4));
            players[client_id]->set_y(lua_tonumber(L, -3));
            players[client_id]->set_z(lua_tonumber(L, -2));
            players[client_id]->set_hp(lua_tointeger(L, -1));
            lua_pop(L, 5);
            players[client_id]->lua_lock.unlock();
            // 부활하는 NPC주변 얘들에게 보이게 해주자
            unordered_set <int> nearlist;
            for (auto& other : players) {
                // if (other._id == client_id) continue;
                if (false == is_near(players[client_id]->get_id(), other->get_id()))
                    continue;
                if (ST_INGAME != other->get_state())
                    continue;
                if (other->get_tribe() != HUMAN) break;
                nearlist.insert(other->get_id());
            }
            for (auto other : nearlist) {
                Player* other_player = reinterpret_cast<Player*>(players[other]);
                other_player->vl.lock();
                other_player->viewlist.insert(client_id);
                other_player->vl.unlock();
                send_put_object_packet(other_player, players[client_id]);
            }
            delete exp_over;
            break;
        }
        case OP_ELEMENT_COOLTIME:

            timer_event ev;

            cout << "여기" << endl;
            cout << "클라 속성 " << players[ev.obj_id]->get_element() << endl;
            cout << "타켓 속성 " << players[ev.target_id]->get_element() << endl;
            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME)) {
                players[client_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                delete exp_over;
                break;
            }
            players[client_id]->state_lock.unlock();
            
            switch (players[client_id]->get_element())  
            {
            case E_WATER:
             
                if(players[exp_over->_target]->get_element() == E_FULLMETAL || players[exp_over->_target]->get_element() == E_FIRE
                    || players[exp_over->_target]->get_element() == E_EARTH)
                players[exp_over->_target]->set_magical_attack(players[exp_over->_target]->get_magical_attack() / 10 * 9);

                cout << "타켓의 마공:"<< players[exp_over->_target]->get_magical_attack() << endl;
                break;
            case E_FULLMETAL:
                break;
            case E_WIND:
                break;
            case E_FIRE:
                break;
            case E_TREE:
                break;
            case E_EARTH:
                break;
            case E_ICE:
                break;
            default:
                break;
            }
             delete exp_over;
            break;

        }
    }
}

// 스크립트 API
int API_get_x(lua_State* L)
{
    int user_id = (int)lua_tointeger(L, -1);
    lua_pop(L, 2);
    int x = players[user_id]->get_x();
    lua_pushnumber(L, x);
    return 1;
}

int API_get_y(lua_State* L)
{
    int user_id =
        (int)lua_tointeger(L, -1);
    lua_pop(L, 2);
    int y = players[user_id]->get_y();
    lua_pushnumber(L, y);
    return 1;
}

int API_get_z(lua_State* L)
{
    int user_id =
        (int)lua_tointeger(L, -1);
    lua_pop(L, 2);
    int z = players[user_id]->get_z();
    lua_pushnumber(L, z);
    return 1;
}

void initialise_NPC()
{
    default_random_engine dre;
    uniform_int_distribution<int> rng_x(1380, 1680);
    uniform_int_distribution<int> rng_z(2070, 2370);
    
    cout << "NPC 로딩중" << endl;
    char name[MAX_NAME_SIZE];

    // 타락한 개구리 소환
    for (int i = NPC_ID_START; i < NPC_ID_START+30; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_flog.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);

        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 나중에 어떻게 이용할 것인지 생각
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);
        
        players[i]->set_mon_species(FALLEN_FLOG);

    }

    // 타락한 닭 소환
    rng_x.param(uniform_int_distribution<int>::param_type(2660, 2960));
    rng_z.param(uniform_int_distribution<int>::param_type(1990, 2290));
    for (int i = NPC_ID_START+30; i < NPC_ID_START + 60; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_chicken.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);

        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 여기는 나중에 생각하자
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);

        players[i]->set_mon_species(FALLEN_CHICKEN);
    }

    // 타락한 토끼 소환
    rng_x.param(uniform_int_distribution<int>::param_type(1900, 2200));
    rng_z.param(uniform_int_distribution<int>::param_type(3080, 3380));
    for (int i = NPC_ID_START + 60; i < NPC_ID_START + 90; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_rabbit.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);

        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 여기는 나중에 생각하자
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);

        players[i]->set_mon_species(FALLEN_RABBIT);
    }
    // 타락한 바나나 원숭이 소환
    rng_x.param(uniform_int_distribution<int>::param_type(3400, 3700));
    rng_z.param(uniform_int_distribution<int>::param_type(2575, 2875));
    for (int i = NPC_ID_START + 90; i < NPC_ID_START + 120; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_monkey.lua") ||
            lua_pcall(L, 0, 0, 0);
        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);
        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 여기는 나중에 생각하자
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);

        players[i]->set_mon_species(FALLEN_MONKEY);
    }
    // 늑대 우두머리 소환
    rng_x.param(uniform_int_distribution<int>::param_type(3125, 3425));
    rng_z.param(uniform_int_distribution<int>::param_type(3210, 3510));
    for (int i = NPC_ID_START + 120; i < NPC_ID_START + 150; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "wolf_boss.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);

        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 여기는 나중에 생각하자
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);

        players[i]->set_mon_species(WOLF_BOSS);
    }

    // 타락한 호랑이 소환
    rng_x.param(uniform_int_distribution<int>::param_type(3020, 3320));
    rng_z.param(uniform_int_distribution<int>::param_type(3622, 3922));
    for (int i = NPC_ID_START + 150; i < NPC_ID_START + 180; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_tiger.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------
        // 여기서 위치를 받아오자

        // 임시 좌표(원래는 몬스터 놓을 곳의 좌표를 뽑아와야한다)
        players[i]->set_x(rng_x(dre));
        players[i]->set_z(rng_z(dre));
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 10, 0);

        if (error != 0) {
            cout << "초기화 오류" << endl;
        }
        players[i]->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_maxhp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));

        lua_pop(L, 10);// eliminate set_uid from stack after call

        // 여기는 나중에 생각하자
        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);

        players[i]->set_mon_species(FALLEN_TIGER);
    }

    cout << "NPC로딩 완료" << endl;
}

#define REAL_DISTANCE 10

int huristic(int t_x, int t_z, int x, int z) 
{
    int s_x = abs(t_x - x);
    int s_z = abs(t_z - z);
    int score = sqrt(pow(s_x, 2) + pow(s_z, 2));
    //cout << "huristic : " << score << endl;
    return score ;
}

// A*알고리즘
pos a_star(int t_x, int t_z, int x, int z)
{
    vector<pos> mon_load;
    // 쫒아가는 범위는 한 방향으로 60까지이다
    int scoreG[25][25] = { 0 };
    int scoreH[25][25] = { 0 };
    int scoreF[25][25] = { 0 };
    pos prior_point[25][25]{ pos(0,0) };

    typedef pair<int, pos> weight;



    pos now(12, 12);
    scoreG[now.first][now.second] = 0;
    scoreH[now.first][now.second] = huristic(t_x, t_z, x, z);
    scoreF[now.first][now.second] = scoreG[now.first][now.second] + scoreH[now.first][now.second];

    priority_queue < weight, vector<weight>, greater<weight>> open_q;
    priority_queue < weight, vector<weight>, greater<weight>> close_q;
    close_q.push(weight(scoreF[now.first][now.second], now));


    int dirX[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
    int dirZ[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
    int cost[8]{ 10, 10, 10, 10, 14, 14, 14, 14 };
    while (true) {
        for (int i = 0; i < 8; i++) {
            pos p(now.first + dirX[i], now.second + dirZ[i]);

            if ((p.first >= 25 || p.first < 0) || (p.second >= 25 || p.second < 0)) continue;
            // 검색된게 있다면 검색을 해주지 않는다
            if (scoreF[now.first + dirX[i]][now.second + dirZ[i]] != 0) continue;
            // 장애물이랑 부딪히는지 확인
            if (false == check_move_alright(x + (p.first - 12) * REAL_DISTANCE, z + (p.second - 12) * REAL_DISTANCE, true)) {
                cout << "장애물 부딪힘" << endl;
                continue;
            }

            scoreG[now.first + dirX[i]][now.second + dirZ[i]] = scoreG[now.first][now.second] + cost[i];
            scoreH[now.first + dirX[i]][now.second + dirZ[i]] = huristic(t_x, t_z, x + (p.first - 12) * REAL_DISTANCE, z + (p.second - 12) * REAL_DISTANCE);
            scoreF[now.first + dirX[i]][now.second + dirZ[i]] = scoreG[now.first + dirX[i]][now.second + dirZ[i]] +
                scoreH[now.first + dirX[i]][now.second + dirZ[i]];

            prior_point[now.first + dirX[i]][now.second + dirZ[i]] = pos(now.first, now.second);

            //cout << "scoreG : " << scoreG[now.first + dirX[i]][now.second + dirZ[i]] << endl;
            //cout << "scoreH : " << scoreH[now.first + dirX[i]][now.second + dirZ[i]] << endl;
            //cout << "scoreF : " << scoreF[now.first + dirX[i]][now.second + dirZ[i]] << endl << endl;

            weight w(scoreF[now.first + dirX[i]][now.second + dirZ[i]], pos(now.first + dirX[i], now.second + dirZ[i]));
            //cout << w.first << ", " << w.second.first << ", " << w.second.second << endl;
            open_q.push(w);
        }
        if (open_q.size() == 0) {
            while (now.first != 12 || now.second != 12) {
                mon_load.push_back(now);
                now = prior_point[now.first][now.second];
            }
            break;
        }
        weight temp = open_q.top();
        open_q.pop();
        now = temp.second;
        close_q.push(temp);

        ///cout << " now : " << now.first << ", " << now.second << endl;

        // 끝내는 조건
        if (abs((x + (now.first - 12) * REAL_DISTANCE) - t_x) <= 10 && abs((z + (now.second - 12) * REAL_DISTANCE) - t_z) <= 10) {
            while (now.first != 12 || now.second != 12) {
                mon_load.push_back(now);
                now = prior_point[now.first][now.second];
            }
            break;
        }
    }

    x += (mon_load.back().first - 12) * REAL_DISTANCE;
    z += (mon_load.back().second - 12) * REAL_DISTANCE;
    mon_load.pop_back();

    return pos(x, z);
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

    pos mv = a_star(my_x, my_z, now_x, now_z);
    if (abs(mv.first-my_x) <=10 && abs(mv.second - my_z) <= 10) {
        now_x = my_x;
        now_z = my_z;
        my_pos_fail = false;
    }
    else {
        now_x = mv.first;
        now_z = mv.second;
    }

    float look_x = players[npc_id]->get_x() - now_x;
    float look_z = players[npc_id]->get_z() - now_z;

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
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
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
    pos mv = a_star(t_x, t_z, x, z);
    x = mv.first;
    z = mv.second;

    float look_x = players[npc_id]->get_x() - x;
    float look_z = players[npc_id]->get_z() - z;

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

    for (auto pl : new_viewlist) {
        // 새로 시야에 들어온 플레이어
        if (0 == old_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.insert(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_put_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
        else {
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
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

    case EVENT_ELEMENT_COOLTIME:
       // return OP_ELEMENT_COOLTIME;
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
                    players[temp.obj_id]->set_physical_attack(0.3 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                    players[temp.obj_id]->set_magical_attack(0.1 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 5 * players[temp.obj_id]->get_lv());
                 
                    //send_status_change_packet(reinterpret_cast<Player*>(players[temp.obj_id]));
                }
       
             
                reinterpret_cast<Player*>(players[temp.obj_id])->set_skill_active(temp.target_id, false);
            }

           
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
        
                if (ev.ev == EVENT_SKILL_COOLTIME) {
                    if (ev.target_id == 2) {  // 전사 BUFF
                        players[ev.obj_id]->set_physical_attack(0.3 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 10 * players[ev.obj_id]->get_lv());
                        players[ev.obj_id]->set_magical_attack(0.1 * players[ev.obj_id]->get_lv() * players[ev.obj_id]->get_lv() + 5 * players[ev.obj_id]->get_lv());
                        // 일단 이것을 넣으면 안돌아감(이유 모름)
                        //send_status_change_packet(reinterpret_cast<Player*>(players[ev.obj_id]));
                    }
            
                    reinterpret_cast<Player*>(players[ev.obj_id])
                        ->set_skill_active(ev.target_id, false);
                    continue;
                }
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
    setlocale(LC_ALL, "korean");
    wcout.imbue(locale("korean"));
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    listen(g_s_socket, SOMAXCONN);

    g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 0, 0);

    // DB 연결 (동시에 DB에 많이 접근하면 DB에서 튕기기 때문)
    InitializeCriticalSection(&cs);

    SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
    EXP_OVER   accept_ex;
    *(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
    ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
    accept_ex._comp_op = OP_ACCEPT;

    AcceptEx(g_s_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
    cout << "Accept Called\n";

    // 초기화 실행
    for (int i = 0; i < MAX_USER; ++i) {
        players[i] = new Player(i);
    }

    // DB 연결1
    // Initialise_DB();
    initialise_NPC();

    ifstream obstacles_read("tree_position.txt");
    if (!obstacles_read.is_open()) {
        cout << "파일을 읽을 수 없습니다" << endl;
        return 0;
    }

    for (int i = 0; i < 609; i++) {
        float x, y, z;
        obstacles_read >> x >> y >> z;
        obstacles[i].set_id(i);
        obstacles[i].set_x(x+100);
        obstacles[i].set_y(y);
        obstacles[i].set_z(z+300);
    }

    obstacles_read.close();

    cout << "중단점" << endl;

    vector <thread> worker_threads;
    thread timer_thread{ do_timer };
    for (int i = 0; i < 16; ++i)
        worker_threads.emplace_back(worker);
    for (auto& th : worker_threads)
        th.join();

    timer_thread.join();
    for (auto& pl : players) {
        if (pl->get_tribe() != HUMAN) break;
        if (ST_INGAME == pl->get_state())
            Disconnect(pl->get_id());
    }
    closesocket(g_s_socket);
    DeleteCriticalSection(&cs);
    WSACleanup();
    
    // DB 연결
    // Disconnect_DB();
}
