#include "stdafx.h"
#include "Player.h"
#include "database.h"
#include "send.h"
#include <fstream>

CRITICAL_SECTION cs;

HANDLE g_h_iocp;
SOCKET g_s_socket;
array <Npc*, MAX_USER + MAX_NPC> players;
array <Obstacle, MAX_OBSTACLE> obstacles;

void do_npc_move(int npc_id, int target);
void return_npc_position(int npc_id);

struct timer_event {
    int obj_id;
    chrono::system_clock::time_point start_time;
    EVENT_TYPE ev;
    /*     target_id
    ��ų ���� ��Ÿ���� ��� : � ��ų���� �־���
    */
    int target_id;      

    constexpr bool operator < (const timer_event& _left) const
    {
        return (start_time > _left.start_time);
    }

};

concurrency::concurrent_priority_queue<timer_event> timer_queue;

//-----------------------------------

//�Ÿ���� �Լ�  //���� ������ 20
/*void element_buf(int c_id, int m_id)
{
    CLIENT& cl = clients[c_id];
    MONSTER& mon = monsters[m_id];
    switch (cl.element)
    {
    case E_WATER: {
        if (mon.element == E_FULLMETAL || mon.element == E_FIRE || mon.element == E_EARTH) {
            mon.nuff_element.buf_setting(B_MAGATTACK, 10.0f, 10.0f);
            mon.nuff_element._use = true;
        }
        break;
    }
    case E_FULLMETAL: {
        if (mon.element == E_ICE || mon.element == E_TREE || mon.element == E_WIND) {
            cl.buff_element.buf_setting(B_PHYDEFENCE, 10.0f, 10.0f);
            cl.buff_element._use = true;
        }
        break;
    }
    case E_WIND: {
        if (mon.element == E_WATER || mon.element == E_EARTH || mon.element == E_FIRE) {
            cl.buff_element.buf_setting(B_SPEED, 5.0f, 6.0f);
            cl.buff_element._use = true;
        }
        break;
    }
    case E_FIRE: {
        if (mon.element == E_ICE || mon.element == E_TREE || mon.element == E_FULLMETAL) {
            mon.nuff_element.buf_setting(B_BURN, mon.physical_attack * 0.1f, 10.0f);
            mon.nuff_element._use = true;
        }
        break;
    }
    case E_TREE: {
        if (mon.element == E_EARTH || mon.element == E_WATER || mon.element == E_WIND) {
            mon.nuff_element.buf_setting(B_PHYATTACK, 10.0f, 10.0f);
            mon.nuff_element._use = true;
        }
        break;
    }
    case E_EARTH: {
        if (mon.element == E_ICE || mon.element == E_FULLMETAL || mon.element == E_FIRE) {
            cl.buff_element.buf_setting(B_MAGDEFENCE, 10.0f, 10.0f);
            cl.buff_element._use = true;
        }
        break;
    }
    case E_ICE: {
        if (mon.element == E_TREE || mon.element == E_WATER || mon.element == E_WIND) {
            mon.nuff_element.buf_setting(B_SPEED, 10.0f, 10.0f);
            mon.nuff_element._use = true;
        }
        break;
    }

    default:
        break;
    }
}
*/

// ���� ��� ����
/*
void send_combat_packet(int c_id, int m_id, TRIBE subject)
{
    CLIENT& cl = clients[c_id];
    MONSTER& mon = monsters[m_id];

    if (subject == T_HUMAN) {	// ��ü�� �޸�
        if (cl.buff_element._use == false)
            element_buf(c_id, m_id);

        // ������ ��� ����
        mon.do_attack = true;
        int damage;
        if (cl.buff_element._type == B_PHYATTACK) damage = (cl.physical_attack*(1+cl.buff_element._effect/100.0f)) * cl.attack_factor;
        else damage = cl.physical_attack * cl.attack_factor;
        float def_temp = mon.defense_factor * mon.physical_defense;
        int real_damage = int(damage * (1.0f - (def_temp) / (1.0f + def_temp)));
        mon.hp -= real_damage;

        // ȭ�鿡 ǥ��
        cout << "�÷��̾� -> ���� ������ : " << real_damage <<  endl;
        cout << c_id << " �÷��̾� Hp : " << cl.hp << endl;
        cout << "���� Hp : " << mon.hp << endl;

        // ������ ���� ������ ��Ŷ�� ��� ������
        sc_packet_attack packet;
        packet.id = c_id;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_ATTACK;
        packet.damage_size = real_damage;
        packet.p_hp = cl.hp;
        packet.m_hp = mon.hp;
        packet.subject = subject;

        cl.do_send(sizeof(packet), &packet);

        if (mon.hp < 0) {
            mon._live = false;
            // ���Ͱ� ��� �������� ������ �Ǿ�� �Ѵ�
            for (auto& cl : clients)
                send_remove_object(cl._id, m_id, T_MONSTER, false);
        }
    }
    else {	// ��ü�� MONSTER

        if (cl.hp > 0) {   //�߰�   ������� ���� �������
            // �Ӽ��ο�
            // ������ ��� ����
            int damage = mon.physical_attack * mon.attack_factor;
            float def_temp;
            if(cl.buff_element._type == B_PHYDEFENCE) def_temp = (cl.defense_factor*(1 + cl.buff_element._effect/100.0f)) * cl.physical_defense;
            else def_temp = cl.defense_factor * cl.physical_defense;
            int real_damage = int(damage * (1.0f - (def_temp) / (1.0f + def_temp)));


            cl.hp -= real_damage;

            if (cl.hp < 0)  //�߰� hp�� ������ 0���� ����
                cl.hp = 0;


            // ȭ�鿡 ǥ��
            cout << "���� -> �÷��̾� ������ : " << real_damage << endl;
            cout << c_id << "�� �÷��̾� Hp : " << cl.hp << endl;  //�߰� ��� �÷��̾ �������޴��� ����
            cout << "���� Hp : " << mon.hp << endl;

            // ������ ���� ������ ��Ŷ�� ��� ������
            sc_packet_attack packet;
            packet.id = c_id;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_ATTACK;
            packet.damage_size = real_damage;
            packet.p_hp = cl.hp;
            packet.m_hp = mon.hp;
            packet.subject = subject;

            cl.do_send(sizeof(packet), &packet);

            if (cl.hp <= 0) {  //�߰�
                mon.do_attack = false;
                //for (auto& cls : clients)
                //	if(cls._use == true)
                //		send_remove_object(cls._id, c_id, T_HUMAN, true);   //���� Ŭ�� ������
                cout << c_id << "�� �÷��̾� ���" << endl;
                cl.hp = 54000;
                cl.x = 0;
                cl.y = 0;
                for (auto& cls : clients) {
                    if (cls._use == true)
                        send_move_packet(cls._id, c_id);
                }
            }
        }

    }

}
*/

//-----------------------------------

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

bool check_move_alright(int x, int z)
{
    for (auto& ob : obstacles) {
        if (ob.get_x() == x && ob.get_z() == z) {
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
        if (true == is_npc(target->get_id())) continue;   // npc�� ��� ������ �ʴ´�
        if (ST_INGAME != target->get_state() && ST_DEAD!=target->get_state()) continue;
        target->vl.lock();
        if (0 != target->viewlist.count(c_id)) {
            target->viewlist.erase(c_id);
            target->vl.unlock();
            send_remove_object_packet(target, players[c_id]);
        }
        else target->vl.unlock();
    }

    // DB ����
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

// ��ũ��Ʈ �߰�
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
            sprintf_s(mess, MAX_CHAT_SIZE, "Kill %s, you get %d experience",
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

        send_status_change_packet(reinterpret_cast<Player*>(players[target]));

        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

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
        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
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
            sprintf_s(mess, MAX_CHAT_SIZE, "Kill %s, you get %d experience",
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
        
        send_status_change_packet(reinterpret_cast<Player*>(players[target]));

        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

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
        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
    }
}

void attack_success(int p_id, int target, float atk_factor)
{
    // ���� ���� ���ݿ� ���ؼ��� �����Ѵ�
    float give_damage = players[p_id]->get_physical_attack() * atk_factor;
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
        if(players[target]->get_state()!=ST_INGAME){
            players[target]->state_lock.unlock();
            return;
        }
        players[target]->set_state(ST_DEAD);
        players[target]->state_lock.unlock();
        if (target < NPC_ID_START) {    // ������ �÷��̾��̴�
            players[p_id]->set_active(false);
            // �������� �÷��̾��� �׾��ٴ� ��Ŷ�� �����ش�
            sc_packet_dead packet;
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_DEAD;
            packet.attacker_id = p_id;
            reinterpret_cast<Player*>(players[target]) ->do_send(sizeof(packet), &packet);
            
            // 3���� ��Ȱ�ϸ� ��Ȱ�� ���ÿ� ��ġ ��ǥ�� �������ش�
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 3s;
            ev.ev = EVENT_PLAYER_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);
        }
        else {  // NPC��� 30�� �Ŀ� ��Ȱ�� �� �ֵ��� ����
            players[target]->set_active(false);
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 30s;
            ev.ev = EVENT_NPC_REVIVE;
            ev.target_id = 0;
            timer_queue.push(ev);

            // �÷��̾�� ����ġ ����, �׸��� �ٲ� ����ġ�� ������ ��������
            int get_exp = players[target]->get_lv() * players[target]->get_lv() * 2;
            if (players[target]->get_tribe() == BOSS)
                get_exp = get_exp * 2;
            char mess[MAX_CHAT_SIZE];
            sprintf_s(mess, MAX_CHAT_SIZE, "Kill %s, you get %d experience",
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
        // ���� target ������ �÷��̾�� ������� ������
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
        // �÷��̾ ������ ���� ���̹Ƿ� hp������ �ٲ�����Ƿ� �װ��� ��������
        send_status_change_packet(reinterpret_cast<Player*>(players[target]));

        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[target]), target, mess);

        // hp�� ���̿����Ƿ� hp�ڵ�ȸ���� ���ֵ��� ����
        if (reinterpret_cast<Player*>(players[target])->_auto_hp == false) {
            timer_event ev;
            ev.obj_id = target;
            ev.start_time = chrono::system_clock::now() + 5s;
            ev.ev = EVENT_AUTO_PLAYER_HP;
            ev.target_id = 0;
            timer_queue.push(ev);
            reinterpret_cast<Player*>(players[target])->_auto_hp = true;
        }

        // npc�����̸� Ÿ�̸� ť�� �ٽ� �־�����
        timer_event ev;
        ev.obj_id = p_id;
        ev.start_time = chrono::system_clock::now() + 3s;
        ev.ev = EVENT_NPC_ATTACK;
        ev.target_id = target;
        timer_queue.push(ev);
    }
    else {  // �÷��̾ ������ ����
        char mess[MAX_CHAT_SIZE];
        sprintf_s(mess, MAX_CHAT_SIZE, "%s -> %s damage : %d",
            players[p_id]->get_name(), players[target]->get_name(), damage);
        send_chat_packet(reinterpret_cast<Player*>(players[p_id]), p_id, mess);
    }
}
struct Coord
{
    float x;
    float y;

};
int calcTriangle(Coord A_TRIANGLE, Coord B_TRIANGLE, Coord C_TRIANGLE)
{
    int result = std::abs((A_TRIANGLE.x * (B_TRIANGLE.y - C_TRIANGLE.y)) +
       ( B_TRIANGLE.x * (C_TRIANGLE.y - A_TRIANGLE.y)) +
        (C_TRIANGLE.x * (A_TRIANGLE.y - B_TRIANGLE.y)));
        return result;
}
bool isInsideTriangle(Coord A_TRIANGLE, Coord B_TRIANGLE, Coord C_TRIANGLE, Coord N)
{
    int NN = calcTriangle(A_TRIANGLE, B_TRIANGLE, C_TRIANGLE);
    int AA = calcTriangle(N, B_TRIANGLE, C_TRIANGLE);
    int BB = calcTriangle(A_TRIANGLE, N, C_TRIANGLE);
    int CC = calcTriangle(A_TRIANGLE, B_TRIANGLE, N);
    return (NN == AA + BB + CC);
}



void process_packet(int client_id, unsigned char* p)
{
    unsigned char packet_type = p[1];
    Player* pl = reinterpret_cast<Player*>(players[client_id]);
    switch (packet_type) {
    case CS_PACKET_LOGIN: {
        cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);

        // pl->set_name(packet->name);

        // DB ����
        /*
        EnterCriticalSection(&cs);
        if (!(Search_Id(pl, packet->name))) {
            send_login_fail_packet(pl, 0);   // ���̵� ����
            Disconnect(client_id);
            LeaveCriticalSection(&cs);
            return;
        }
        LeaveCriticalSection(&cs);
        */

        // �ߺ� ���̵� �˻�
        /*
        for (auto* p : players) {
            if (p->get_tribe() != HUMAN) break;
            if (p->get_state() == ST_FREE) continue;
            if (p->get_id() == client_id) continue;
            if (reinterpret_cast<Player*>(p)->get_login_id() == pl->get_login_id()) {
                send_login_fail_packet(pl, 1);   // �ߺ� �α���
                Disconnect(client_id);
                return;
            }
        }
        */
        // ������ DB���� �޾ƿ;� �ϴ� ������ �⺻ ������ ��ü
        pl->set_x(300);
        pl->set_y(0);
        pl->set_z(300);
        pl->set_job(J_DILLER);
        pl->set_lv(25);
        switch (pl->get_job()) {
        case J_DILLER: {
            int lv = pl->get_lv();
            pl->set_maxhp(20 * lv * lv + 80 * lv);
            pl->set_hp(pl->get_maxhp());
            //pl->set_maxmp(10*lv*lv + 50*lv);
            pl->set_physical_attack(0.3 * lv * lv + 10 * lv);
            pl->set_magical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_physical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);

            for (int i = 0; i < 10; ++i) {
                pl->set_skill_factor(0, i);
                pl->set_skill_factor(1, i);
              //  pl->set_skill_factor(2, i);
            }

            break;
        }
        default: {
            cout << "���� ����" << endl;
            break;
        }
        }
        // -- DB ��ü �� --

        // Hpȸ��
        if (pl->get_hp() <= pl->get_maxhp()) {
            // hp�� ���̿����Ƿ� hp�ڵ�ȸ���� ���ֵ��� ����
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

        // ���� ������ ������ �ٸ��̿��� ������
        for (auto& other : players) {
            if (other->get_id() == client_id) continue;   // ����

            if (true == is_npc(other->get_id())) break;// ���� ���� �ִ� ���� NPC�� �ִٸ�

            other->state_lock.lock();
            if (ST_INGAME != other->get_state()) {
                other->state_lock.unlock();
                continue;
            }
            other->state_lock.unlock();

            if (false == is_near(other->get_id(), client_id)) continue;

            // ����� �÷��̾� ó��
            Player* other_player = reinterpret_cast<Player*>(other);
            other_player->vl.lock();
            other_player->viewlist.insert(client_id);
            other_player->vl.unlock();
            sc_packet_put_object packet;
            packet.id = client_id;
            strcpy_s(packet.name, pl->get_name());
            packet.object_type = pl->get_tribe();
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_PUT_OBJECT;
            packet.x = pl->get_x();
            packet.y = pl->get_y();
            packet.z = pl->get_z();
            other_player->do_send(sizeof(packet), &packet);
        }
        // ���� ������ �÷��̾�� ���� ������ ������
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

            // ��ũ��Ʈ�� �Բ� �߰��� �κ�
            if (true == is_npc(other->get_id())) {	// �þ߿� npc�� �ִٸ� 
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

            sc_packet_put_object packet;
            packet.id = other->get_id();
            strcpy_s(packet.name, other->get_name());
            packet.object_type = other->get_tribe();
            packet.size = sizeof(packet);
            packet.type = SC_PACKET_PUT_OBJECT;
            packet.x = other->get_x();
            packet.y = other->get_y();
            packet.z = other->get_z();
            pl->do_send(sizeof(packet), &packet);
        }
        // ��ֹ� ����
        for (auto& ob : obstacles) {
            if (RANGE < abs(pl->get_x() - ob.get_x())) continue;
            if (RANGE < abs(pl->get_z() - ob.get_z())) continue;

            pl->ob_vl.lock();
            pl->ob_viewlist.clear();
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
        break;
    }
    case CS_PACKET_MOVE: {
        cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
        // pl.last_move_time = packet->move_time;
        float x = pl->get_x();
        float y = pl->get_y();
        float z = pl->get_z();
        pl->last_move_time = packet->move_time;
        pl->direction = packet->direction;
        switch (packet->direction) {
        case 0: {
            x = pl->get_look_x() * PLAYER_VELOCITY + x;
            z = pl->get_look_z() * PLAYER_VELOCITY + z;
            break;
        }
        case 1: {
            x = -(pl->get_look_x()) * PLAYER_VELOCITY + x;
            z = -(pl->get_look_z()) * PLAYER_VELOCITY + z;
            break;
        }
        case 2: {
            x = -(pl->get_right_x()) * PLAYER_VELOCITY + x;
            z = -(pl->get_right_z()) * PLAYER_VELOCITY + z;
            break;
        }
        case 3: {
            x = (pl->get_right_x()) * PLAYER_VELOCITY + x;
            z = (pl->get_right_z()) * PLAYER_VELOCITY + z;
            break;
        }
        default:
            cout << "Invalid move in client " << client_id << endl;
            exit(-1);
        }
        if (check_move_alright(x, z) == false) {
            break;
        }

        pl->set_x(x);
        pl->set_z(z);

        unordered_set <int> nearlist;
        for (auto& other : players) {
            // if (other._id == client_id) continue;
            if (false == is_near(client_id, other->get_id()))
                continue;
            if (ST_INGAME != other->get_state())
                continue;
            //��ũ��Ʈ �߰�
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
        nearlist.erase(client_id);  // �� ���̵�� ������ ���� �װ��� ��������

        send_move_packet(pl, pl); // �� �ڽ��� �������� ���� ��������

        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

        // ���νþ߿� ���� �÷��̾� ó��
        for (auto other : nearlist) {
            if (0 == my_vl.count(other)) {   // ���� ���� �÷��̾�/npc
                pl->vl.lock();
                pl->viewlist.insert(other);
                pl->vl.unlock();
                send_put_object_packet(pl, players[other]);

                // ��ũ��Ʈ �߰�
                if (true == is_npc(other)) break;

                // ����� �÷��̾� ó���̴�.
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
            // ��� �þ߿� �����ϴ� �÷��̾� ó��
            else {
                if (true == is_npc(other)) continue;   // ���� �ִ� npc�� npc_move���� ó��

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
        // �þ߿��� ����� �÷��̾� ó��
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
        // ���� ���� ��ֹ��� ���� ����
        // ��ֹ� ����

        for (auto& ob : obstacles) {
            if ((RANGE < abs(pl->get_x() - ob.get_x())) &&
                (RANGE < abs(pl->get_z() - ob.get_z()))) {
                // ���� �������(�����ϴ��� ������ ������)
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
            // �̹� �����ϴ°�
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
        // �÷��̾ �����ϰ� �ݰ� 1ĭ �̳��� ���Ͱ� �ִٸ� ����
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
            if (players[i]->get_x() >= pl->get_x() - 10 && players[i]->get_x() <= pl->get_x() + 10) {
                if (players[i]->get_z() >= pl->get_z() - 10 && players[i]->get_z() <= pl->get_z() + 10) {
                    attack_success(client_id, players[i]->get_id(), pl->get_basic_attack_factor());    // ������ ���
                    // ������ �ڵ������� �־�����
                    if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                        players[i]->set_active(true);
                        ev.obj_id = i;
                        ev.start_time = chrono::system_clock::now() + 1s;
                        ev.ev = EVENT_NPC_ATTACK;
                        ev.target_id = client_id;
                        timer_queue.push(ev);
                        // ������ �̵��� �־�����
                        Activate_Npc_Move_Event(i, pl->get_id());
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
        pl->direction = 1;
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
    case CS_PACKET_SKILL:{
        cs_packet_skill* packet = reinterpret_cast<cs_packet_skill*>(p);
        if (pl->get_skill_active(packet->skill_type) == true) return;
        pl->set_skill_active(packet->skill_type, true);     //일반공격 계수는 50
        switch (packet->skill_type)
        {
        case 0:    // 물리 공격 스킬 
            switch (packet->skill_num)
            {
            case 0:  //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                ev.ev = EVENT_PSKILL_COOLTIME;
                ev.target_id = 0;
                timer_queue.push(ev);

                for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                    players[i]->state_lock.lock();
                    if (players[i]->get_state() != ST_INGAME) {
                        players[i]->state_lock.unlock();
                        continue;
                    }
                    players[i]->state_lock.unlock();
                    if ((players[i]->get_x() >= pl->get_x() - 10 && players[i]->get_x() <= pl->get_x() + 10 )|| (players[i]->get_z() >= pl->get_z() - 10 && players[i]->get_z() <= pl->get_z() + 10)) {
                        physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                            cout << "최후의 일격 !!!" << endl;
                            if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                players[i]->set_active(true);
                                timer_event ev;
                                ev.obj_id = i;
                                ev.start_time = chrono::system_clock::now() + 1s;
                                ev.ev = EVENT_NPC_ATTACK;
                                ev.target_id = client_id;
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
                ev.ev = EVENT_MSKILL_COOLTIME;
                ev.target_id = 1;
                timer_queue.push(ev);

                Coord a = { pl->get_x(), pl->get_z() };    //플레이어 기준 전방 삼각형 범위 
                Coord b = { pl->get_x() - 100, pl->get_z() + 40 };
                Coord c = { pl->get_x() + 100, pl->get_z()  + 40};


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
                        magical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                        cout << "광야 일격 !!!" << endl;
                        if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                            players[i]->set_active(true);
                            timer_event ev;
                            ev.obj_id = i;
                            ev.start_time = chrono::system_clock::now() + 1s;
                            ev.ev = EVENT_NPC_ATTACK;
                            ev.target_id = client_id;
                            timer_queue.push(ev);

                            Activate_Npc_Move_Event(i, pl->get_id());
                        }
                    }
                }
                break;
            }
            
            break;
        case 2:  //버프 
            switch (packet->skill_num)
            {
            case 0:
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
                ev.ev = EVENT_BUFF_COOLTIME;
                ev.target_id = 2;
                timer_queue.push(ev);

   
                    cout << pl->get_physical_defence() << endl;
                    cout << pl->get_magical_defence() << endl;
                    cout << "아테네의 가호 !!!" << endl;
                    pl->set_physical_defence(0.48 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv()); //일단 두배 
                    pl->set_magical_defence(0.34 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv());

                    send_status_change_packet(pl);
                    cout << pl->get_physical_defence() << endl;
                    cout << pl->get_magical_defence() << endl;
                    //여기 타이머 해야함 

                
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

  
        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

 
        cout << pl->get_look_x() << ", " << pl->get_look_y() << ", " << pl->get_look_z() << endl;

        for (auto i : my_vl) {
            // Npc
            if (is_npc(i) == true) continue;
            // Player
            sc_packet_look s_packet;
            s_packet.size = sizeof(s_packet);
            s_packet.type = SC_PACKET_LOOK;
            s_packet.id = pl->get_id();
            s_packet.x = pl->get_look_x();
            s_packet.y = pl->get_look_y();
            s_packet.z = pl->get_look_z();
            



            reinterpret_cast<Player*>(players[i])->do_send(sizeof(s_packet), &s_packet);
        }
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

    pl->set_hp(players[client_id]->get_maxhp());
    pl->set_x(0);
    pl->set_y(0);
    pl->set_z(0);
    pl->set_exp(pl->get_exp() / 2);
    send_status_change_packet(pl);

    sc_packet_revive packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_REVIVE;
    packet.x = pl->get_x();
    packet.y = pl->get_y();
    packet.z = pl->get_z();
    packet.hp = pl->get_hp();
    packet.exp = pl->get_exp();
    pl->do_send(sizeof(packet), &packet);

    pl->vl.lock();
    pl->viewlist.clear();
    pl->vl.unlock();
    for (auto& other : players) {
        if (other->get_id() == client_id) continue;  
        if (true == is_npc(other->get_id())) break;
        other->state_lock.lock();
        if (ST_INGAME != other->get_state()) {
            other->state_lock.unlock();
            continue;
        }
        other->state_lock.unlock();
        if (false == is_near(other->get_id(), client_id)) continue;


        Player* other_player = reinterpret_cast<Player*>(other);
        other_player->vl.lock();
        other_player->viewlist.insert(client_id);
        other_player->vl.unlock();
        sc_packet_put_object packet;
        packet.id = client_id;
        strcpy_s(packet.name, pl->get_name());
        packet.object_type = 0;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_PUT_OBJECT;
        packet.x = pl->get_x();
        packet.y = pl->get_y();
        packet.z = pl->get_z();
        other_player->do_send(sizeof(packet), &packet);
    }


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

        pl->vl.lock();
        pl->viewlist.insert(other->get_id());
        pl->vl.unlock();

        sc_packet_put_object packet;
        packet.id = other->get_id();
        strcpy_s(packet.name, other->get_name());
        packet.object_type = 0;
        packet.size = sizeof(packet);
        packet.type = SC_PACKET_PUT_OBJECT;
        packet.x = other->get_x();
        packet.y = other->get_y();
        packet.z = other->get_z();
        pl->do_send(sizeof(packet), &packet);
    }

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

            if (exp_over->_target == -1) {
                return_npc_position(client_id);
                delete exp_over;
                break;
            }
            int target_id = exp_over->_target;
            players[target_id]->state_lock.lock();
    
            if (players[target_id]->get_state() != ST_INGAME) {
                players[target_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                return_npc_position(client_id);
                delete exp_over;
                break;
            }
            players[target_id]->state_lock.unlock();

            players[client_id]->lua_lock.lock();
            lua_State* L = players[client_id]->L;
            lua_getglobal(L, "event_npc_move");
            lua_pushnumber(L, exp_over->_target);
            int error = lua_pcall(L, 1, 1, 0);
            if (error != 0) {
                cout << "LUA_NPC_MOVE ERROR" << endl;
            }
       
            bool m = lua_toboolean(L, -1);
            lua_pop(L, 1);
            players[client_id]->lua_lock.unlock();
            if (m) {
                do_npc_move(client_id, exp_over->_target);
            }
            else {

                players[client_id]->set_active(false);
                return_npc_position(client_id);
            }
            delete exp_over;
            
            break;
        }
        case OP_NPC_ATTACK: {

            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME) || (false == players[client_id]->get_active())) {
                players[client_id]->state_lock.unlock();
                delete exp_over;
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

                attack_success(client_id, exp_over->_target, players[client_id]->get_basic_attack_factor());
            }
            else {
                if (players[client_id]->get_active()) {
               
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
            /*
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
            send_status_change_packet(pl);
            */
            break;
        }
        case OP_PLAYER_REVIVE: {
            player_revive(client_id);
            delete exp_over;
            break;
        }
        case OP_NPC_REVIVE: {
 
            players[client_id]->state_lock.lock();
            players[client_id]->set_state(ST_INGAME);
            players[client_id]->state_lock.unlock();
       
            players[client_id]->lua_lock.lock();
            lua_State* L = players[client_id]->L;
            lua_getglobal(L, "monster_revive");
            int error = lua_pcall(L, 0, 4, 0);
            if (error != 0) {
                cout << "ㄱㄹ" << endl;
            }

            players[client_id]->set_x(lua_tonumber(L, -4));
            players[client_id]->set_y(lua_tonumber(L, -3));
            players[client_id]->set_z(lua_tonumber(L, -2));
            players[client_id]->set_hp(lua_tointeger(L, -1));
            lua_pop(L, 5);
            players[client_id]->lua_lock.unlock();

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
        }
    }
}


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
    cout << "NPC" << endl;
    char name[MAX_NAME_SIZE];

    for (int i = NPC_ID_START; i < NPC_ID_START+30; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_flog.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------

        players[i]->set_x((i-1000)*10 + 301);
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 9, 0);

        if (error != 0) {
            cout << "ㅁㄴㅇ" << endl;
        }
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call


        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);
     
        
        players[i]->set_mon_species(FALLEN_FLOG);

    }

    for (int i = NPC_ID_START+30; i < NPC_ID_START + 60; ++i) {
        players[i] = new Npc(i);
        lua_State* L = players[i]->L = luaL_newstate();
        luaL_openlibs(L);
        int error = luaL_loadfile(L, "fallen_chicken.lua") ||
            lua_pcall(L, 0, 0, 0);

        //-------------------------------------------

        players[i]->set_x((i - 1000) * 10 + 601);
        float temp_x = players[i]->get_x();
        float temp_y = players[i]->get_y();
        float temp_z = players[i]->get_z();
        //-------------------------------------------


        lua_getglobal(L, "set_uid");
        lua_pushnumber(L, i);
        lua_pushnumber(L, temp_x);
        lua_pushnumber(L, temp_y);
        lua_pushnumber(L, temp_z);
        error = lua_pcall(L, 4, 9, 0);

        if (error != 0) {
            cout << "121" << endl;
        }
        players[i]->set_lv(lua_tointeger(L, -9));
        players[i]->set_name(lua_tostring(L, -8));
        players[i]->set_hp(lua_tointeger(L, -7));
        players[i]->set_physical_attack(lua_tonumber(L, -6));
        players[i]->set_magical_attack(lua_tonumber(L, -5));
        players[i]->set_physical_defence(lua_tonumber(L, -4));
        players[i]->set_magical_defence(lua_tonumber(L, -3));
        players[i]->set_basic_attack_factor(lua_tointeger(L, -2));
        players[i]->set_defence_factor(lua_tonumber(L, -1));
        lua_pop(L, 10);// eliminate set_uid from stack after call

        lua_register(L, "API_get_x", API_get_x);
        lua_register(L, "API_get_y", API_get_y);
        lua_register(L, "API_get_z", API_get_z);
    

        players[i]->set_mon_species(FALLEN_CHICKEN);
    }


    cout << "NPC" << endl;
}

void return_npc_position(int npc_id)
{
    if (players[npc_id]->get_active() == true) {
        return;
    }
    unordered_set<int> old_viewlist;
    unordered_set<int> new_viewlist;
    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;
        // if (true == is_npc(obj._id)) continue;   // 
        if (true == is_npc(obj->get_id())) break; 
        if (true == is_near(npc_id, obj->get_id())) {     
            old_viewlist.insert(obj->get_id());         // npc
        }
    }

  
    players[npc_id]->lua_lock.lock();
    lua_State* L = players[npc_id]->L;
    lua_getglobal(L, "return_my_position");
    int error = lua_pcall(L, 0, 3, 0);  //?
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
    if (my_x != now_x) {
        if (my_x >= now_x) now_x = now_x + 5;
        else now_x = now_x - 5;
    }
    else if (my_z != now_z) {
        if (my_z >= now_z) now_z = now_z + 5;
        else now_z = now_z - 5;
    }
    else my_pos_fail = false;

    if (false == check_move_alright(now_x, now_z)) {
        return;
    }
    players[npc_id]->set_x(now_x);
    players[npc_id]->set_z(now_z);


    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;   // in game�� �ƴҶ�
        //if (true == is_npc(obj._id)) continue;   // npc�� �ƴҶ� -> ingame���� �÷��̾� ã��
        if (true == is_npc(obj->get_id())) break;   // npc�� �ƴҶ� -> ingame���� �÷��̾� ã��
        if (true == is_near(npc_id, obj->get_id())) {
            new_viewlist.insert(obj->get_id());
        }
    }

    for (auto pl : new_viewlist) {
        // ���� �þ߿� ���� �÷��̾�
        if (0 == old_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.insert(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_put_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
  
        }
        else {
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);

        }
    }

    // �þ߿� ����� ���
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

    if (my_pos_fail) {    // �� �������ߵ�
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
    unordered_set<int> old_viewlist;
    unordered_set<int> new_viewlist;
    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;
        if (true == is_npc(obj->get_id())) break;   // npc�� �ƴҶ�
        if (true == is_near(npc_id, obj->get_id())) {      // ��ó�� ������
            old_viewlist.insert(obj->get_id());         // npc��ó�� �÷��̾ ������ old_viewlist�� �÷��̾� id�� �ִ´�
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
    
    cout << "Move : " << x << "," << z << endl;
    // ������ ���⿡ A*�˰������ �־�� �Ѵ�
    if (t_x != x) {
        if (t_x > x) x+=5;
        else x-=5;
    }
    else if(t_z != z){
        if (t_z > z) z+=5;
        else z-=5;
    }
    
    if (false == check_move_alright(x, z)) {
        return;
    }
    players[npc_id]->set_x(x);
    players[npc_id]->set_z(z);

    for (auto& obj : players) {
        if (obj->get_state() != ST_INGAME) continue;   // in game�� �ƴҶ�
        //if (true == is_npc(obj._id)) continue;   // npc�� �ƴҶ� -> ingame���� �÷��̾� ã��
        if (true == is_npc(obj->get_id())) break;   // npc�� �ƴҶ� -> ingame���� �÷��̾� ã��
        if (true == is_near(npc_id, obj->get_id())) {
            new_viewlist.insert(obj->get_id());
        }
    }

    for (auto pl : new_viewlist) {
        // ���� �þ߿� ���� �÷��̾�
        if (0 == old_viewlist.count(pl)) {
            reinterpret_cast<Player*>(players[pl])->vl.lock();
            reinterpret_cast<Player*>(players[pl])->viewlist.insert(npc_id);
            reinterpret_cast<Player*>(players[pl])->vl.unlock();
            send_put_object_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
        else {
            send_move_packet(reinterpret_cast<Player*>(players[pl]), players[npc_id]);
        }
    }

    // �þ߿� ����� ���
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
            else if (temp.ev == EVENT_PSKILL_COOLTIME  ) {
                reinterpret_cast<Player*>(players[temp.obj_id])
                    ->set_skill_active(temp.target_id, false);              
            }
            else if (temp.ev == EVENT_MSKILL_COOLTIME) {
                reinterpret_cast<Player*>(players[temp.obj_id])
                    ->set_skill_active(temp.target_id, false);
            }
            else if (temp.ev == EVENT_BUFF_COOLTIME) {

                players[temp.obj_id]->set_physical_defence(0.24 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                players[temp.obj_id]->set_magical_defence(0.17 * players[temp.obj_id]->get_lv() * players[temp.obj_id]->get_lv() + 10 * players[temp.obj_id]->get_lv());
                send_status_change_packet(reinterpret_cast<Player*>(players[temp.obj_id]));

                cout << players[temp.obj_id]->get_physical_defence() << endl;
                cout << players[temp.obj_id]->get_magical_defence() << endl;
                reinterpret_cast<Player*>(players[temp.obj_id])
                    ->set_skill_active(temp.target_id, false);
         
            }
            else {
                EXP_OVER* ex_over = new EXP_OVER;
                ex_over->_comp_op = EVtoOP(temp.ev);
                ex_over->_target = temp.target_id;
                PostQueuedCompletionStatus(g_h_iocp, 1, temp.obj_id, &ex_over->_wsa_over);   //0�� ��������� ����
            }
        }

        while (true) {
            timer_event ev;
            if (timer_queue.size() == 0) break;
            timer_queue.try_pop(ev);

            dura = ev.start_time - chrono::system_clock::now();
            if (dura <= 0ms) {
                EXP_OVER* ex_over = new EXP_OVER;
                if (ev.ev == EVENT_PLAYER_ATTACK) {
                    reinterpret_cast<Player*>(players[ev.obj_id])->set_attack_active(false);
                    continue;
                }
                else if (temp.ev == EVENT_PSKILL_COOLTIME) {
                    reinterpret_cast<Player*>(players[temp.obj_id])
                        ->set_skill_active(temp.target_id, false);
                    continue;
                }
                else if (temp.ev == EVENT_MSKILL_COOLTIME) {
                    reinterpret_cast<Player*>(players[temp.obj_id])
                        ->set_skill_active(temp.target_id, false);
                    continue;
                }
                else if (temp.ev == EVENT_BUFF_COOLTIME) {
                    reinterpret_cast<Player*>(players[temp.obj_id])
                        ->set_skill_active(temp.target_id, false);
              
                    continue;
                }
                ex_over->_comp_op = EVtoOP(ev.ev);
                ex_over->_target = ev.target_id;
                PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);   //0�� ��������� ����
            }
            else if (dura <= waittime) {
                temp = ev;
                temp_bool = true;
                break;
            }
            else {
                timer_queue.push(ev);   // Ÿ�̸� ť�� ���� �ʰ� ����ȭ �ʿ�
            }
        }
        this_thread::sleep_for(dura);
        // �� �翩�־ ��� ó���� �ϵ��� �ؾ���
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

    // DB ���� (���ÿ� DB�� ���� �����ϸ� DB���� ƨ��� ����)
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

    // �ʱ�ȭ ����
    for (int i = 0; i < MAX_USER; ++i) {
        players[i] = new Player(i);
    }

    // DB ����1
    // Initialise_DB();
    initialise_NPC();

    ifstream obstacles_read("tree_position.txt");
    if (!obstacles_read.is_open()) {

        return 0;
    }

    for (int i = 0; i < 609; i++) {
        float x, y, z;
        obstacles_read >> x >> y >> z;
        cout << x << "," << y << "," << z << endl;
        obstacles[i].set_id(i);
        obstacles[i].set_x(x);
        obstacles[i].set_y(y);
        obstacles[i].set_z(z);
    }

    obstacles_read.close();

    cout << "�ߴ���" << endl;

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
    
    // DB ����
    // Disconnect_DB();
}
