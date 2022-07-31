#include "PacketManager.h"
#include "send.h"
#include "AllJobHeader.h"
#include "TimerManager.h"
#include "database.h"

//bool check_inside(Coord a, Coord b, Coord c, Coord n) {
//    Coord A, B, C;
//    A.x = b.x - a.x;
//    A.z = b.z - a.z;
//    B.x = c.x - a.x;
//    B.z = c.z - a.z;
//    C.x = n.x - a.x;
//    C.z = n.z - a.z;
//
//    if ((A.x * B.z - A.z * B.x) * (A.x * C.z - A.z * C.x) < 0)
//        return false;
//    return true;
//}
//
//bool isInsideTriangle(Coord a, Coord b, Coord c, Coord n)
//{
//    if (!check_inside(a, b, c, n)) return false;
//    if (!check_inside(b, c, a, n)) return false;
//    if (!check_inside(c, a, b, n)) return false;
//    return true;
//}

PacketManager::PacketManager(ObjectManager* objectManager, SectorManager* sectorManager, HANDLE* iocp)
{
	m_ObjectManger = objectManager;
    m_SectorManager = sectorManager;
    h_iocp = iocp;
}

void PacketManager::set_players_object(array <Npc*, MAX_USER + MAX_NPC + MAX_AI>& pls)
{
    players = pls;
}

void PacketManager::process_packet(Player* pl, unsigned char* p)
{

    unsigned char packet_type = p[1];
    int client_id = pl->get_id();
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

        for (const auto& p : players) {
            if (p->get_tribe() != HUMAN) break;
            if (p->get_state() == ST_FREE) continue;
            if (p->get_id() == client_id) continue;
            if (strcmp(packet->id, "admin") == 0) break;

            if (strcmp(reinterpret_cast<Player*>(p)->get_login_id(), packet->id) == 0) {
                cout << "중복된 아이디 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
               // m_ObjectManger->Disconnect(client_id);
                return;
            }
            if (strcmp(reinterpret_cast<Player*>(p)->get_name(), packet->name) == 0) {
                cout << "중복된 닉네임 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
               // m_ObjectManger->Disconnect(client_id);
                return;
            }

        }
        // 데이터 베이스
        pl->set_login_id(packet->id);
        bool login = false;
        //데이터 베이스 
        if (DB_On) {
            login = Search_Id(pl, packet->id, packet->password);
            if (login == false) {
                send_login_fail_packet(pl, 1);  // 아이디 비번 일치 계정 없음! 보내줘! 
            }
        }
        else {
            pl->set_x(3210);
            pl->set_y(0);
            pl->set_z(940);
            pl->set_job(static_cast<JOB>(packet->job));
            //pl->set_job(J_DILLER);
            pl->set_lv(25);
            pl->set_element(E_WATER);
            pl->set_exp(1000);
            pl->set_name(packet->name);
            pl->set_login_id(packet->id);

            pl->indun_id - 1;
            pl->join_dungeon_room = false;
            login = true;
        }

        // Stress Test용
        if (strcmp(packet->id, "admin") == 0) {
            pl->set_x(rand() % 4000);
            pl->set_z(rand() % 4000);
        }

        switch (pl->get_job()) {
        case J_DILLER: {
            Diller::Initialize(pl);
            break;
        }
        case J_TANKER: {
            Tanker::Initialize(pl);
            break;
        }
        case J_SUPPORTER: {
            Supporter::Initialize(pl);
            break;
        }
        case J_MAGICIAN: {
            Magician::Initialize(pl);
            break;
        }
        default: {
            break;
        }
        }
        // -- DB 대체 끝 --

        // Hp회복
        if (pl->get_hp() < pl->get_maxhp()) {
            // hp가 깎이였으므로 hp자동회복을 해주도록 하자
            if (pl->_auto_hp == false) {
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;
                ev.ev = EVENT_AUTO_PLAYER_HP;
                ev.target_id = 0;
                TimerManager::timer_queue.push(ev);
                pl->_auto_hp = true;
            }
        }
        else pl->_auto_hp = false;

        if(login == true) send_login_ok_packet(pl);
        pl->state_lock.lock();
        pl->set_state(ST_INGAME);
        pl->state_lock.unlock();

        m_SectorManager->player_put(pl);
        Save_position(pl);

        break;
    }
    case CS_PACKET_MOVE: {
        cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
        // pl.last_move_time = packet->move_time;
        pl->last_move_time = packet->move_time;
        float x = packet->x;
        float y = packet->y;
        float z = packet->z;

        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();
        // InDunProcess
        if (pl->get_state() == ST_INDUN) {
            // 유효성 검사
            if (m_ObjectManger->check_move_alright_indun(x, z) == false) { // Raid Map에 맞는 유효성 검사 필요
                send_move_packet(pl, pl, 0);   
                break;
            }
            pl->set_x(x);
            pl->set_y(y);
            pl->set_z(z);
            pl->vl.lock();
            unordered_set <int> my_vl{ pl->viewlist };
            pl->vl.unlock();
            send_move_packet(pl, pl, 1);

            for (int vl_id : my_vl) {
                send_move_packet(reinterpret_cast<Player*>(players[vl_id]), pl, 1);
            }
            break;
        }


        // 유효성 검사
        if (m_ObjectManger->check_move_alright(x, z, false) == false) {
            // 올바르지 않을경우 위치를 수정을 해주어야 한다
            send_move_packet(pl, pl, 0);
            break;
        }

        pl->set_x(x);
        pl->set_y(y);
        pl->set_z(z);
        if (pl->get_state() != ST_FREE)
            m_SectorManager->player_move(pl);
        break;
    }
    case CS_PACKET_ATTACK: {
        // cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(p);
        // 플레이어가 공격하고 반경 1칸 이내에 몬스터가 있다면 전투

        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        if (pl->get_attack_active()) break;

        pl->set_attack_active(true);
        send_animation_attack(pl, pl->get_id());
        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();
        for (auto vl_id : my_vl) {
            if (m_ObjectManger->get_player(vl_id)->get_tribe() == HUMAN) {
                send_animation_attack(reinterpret_cast<Player*>(m_ObjectManger->get_player(vl_id)), pl->get_id());
            }
        }

        timer_event ev;
        if (pl->attack_speed_up == 0) {
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_PLAYER_ATTACK;
            ev.target_id = client_id;
            TimerManager::timer_queue.push(ev);
        }
        else if (pl->attack_speed_up == 1) {
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 770ms;
            ev.ev = EVENT_PLAYER_ATTACK;
            ev.target_id = client_id;
            TimerManager::timer_queue.push(ev);
        }
        else if (pl->attack_speed_up == -1) {
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 1s + 300ms;
            ev.ev = EVENT_PLAYER_ATTACK;
            ev.target_id = client_id;
            TimerManager::timer_queue.push(ev);
        }
        if (pl->join_dungeon_room) {
            //int indun_id = pl->indun_id;
            Gaia* indun = m_ObjectManger->get_dungeon(pl->indun_id);
            Npc* bos = indun->boss;
            if (bos->get_x() >= pl->get_x() - 20 && bos->get_x() <= pl->get_x() + 20) {
                if (bos->get_z() >= pl->get_z() - 20 && bos->get_z() <= pl->get_z() + 20) {
                    // 일단 고정값으로 제거해 주자
                    //bos->set_hp(bos->get_hp() - 130000);
                    if (bos->get_hp() > 0) {
                        pl->basic_attack_success(bos);

                        Player** ps = indun->get_party_palyer();
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            send_change_hp_packet(ps[i], bos, 0);
                        }

                        if (bos->get_hp() <= 0) {
                            bos->set_hp(0);
                            indun->game_victory();
                        }
                    }

                }
            }
            return;
        }

        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
            Npc* n = m_ObjectManger->get_player(i);
            n->state_lock.lock();
            if (n->get_state() != ST_INGAME) {
                n->state_lock.unlock();
                continue;
            }
            n->state_lock.unlock();
            if (n->get_x() >= pl->get_x() - 20 && n->get_x() <= pl->get_x() + 20) {
                if (n->get_z() >= pl->get_z() - 20 && n->get_z() <= pl->get_z() + 20) {
                    pl->basic_attack_success(n);

                    // 죽었다면 섹터에서 제거해 주어야 함
                    n->state_lock.lock();
                    if (n->get_state() == ST_DEAD) {
                        n->state_lock.unlock();
                        m_SectorManager->player_remove(n, true, players[client_id]);
                    }
                    else {  // target의 피 변화량을 주위 사람들에게 보내주어야함
                        n->state_lock.unlock();
                    }


                    // 몬스터의 자동공격을 넣어주자
                    n->set_target_id(pl->get_id());
                    if (n->get_active() == false && n->get_tribe() == MONSTER) {
                        n->set_active(true);
                        ev.obj_id = i;
                        ev.start_time = chrono::system_clock::now() + 1s;
                        ev.ev = EVENT_NPC_ATTACK;
                        ev.target_id = n->get_target_id();
                        TimerManager::timer_queue.push(ev);
                        // 몬스터의 이동도 넣어주자
                        //n->push_npc_move_event();
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
        if (pl->join_dungeon_room) {
            if (m_ObjectManger->get_dungeon(pl->get_indun_id())->get_dun_st() == DUN_ST_START) {
                Player** vl_pl;
                vl_pl = m_ObjectManger->get_dungeon(pl->get_indun_id())->get_party_palyer();
                for (int i = 0; i < GAIA_ROOM; i++) {
                    if (vl_pl[i]->get_state() == ST_INGAME || vl_pl[i]->get_state() == ST_INDUN) {
                        if(vl_pl[i]->get_tribe() != PARTNER)
                            send_chat_packet(vl_pl[i], client_id, c_temp);
                    }
                }
                break;
            }
        }
        else {
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
        }
        break;
    }
    case CS_PACKET_TELEPORT: {
        pl->set_x(rand() % WORLD_WIDTH);
        //pl->set_y(rand() % WORLD_HEIGHT);
        pl->set_z(rand() % WORLD_HEIGHT);
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
        if ((pl->get_mp() - pl->get_lv() * 10 < 0 ) || ( (pl->get_mp() - pl->get_lv() * 10 < 0) && (pl->get_hp() - pl->get_lv() * 10 < 0)) ) //mp, hp없으면 안됨 
            return;
        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        cs_packet_skill* packet = reinterpret_cast<cs_packet_skill*>(p);
        if (pl->get_skill_active((int)packet->skill_type) == true) return;
        pl->set_skill_active((int)packet->skill_type, true);     //일반공격 계수는 5

        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

        switch (pl->get_job())
        {
        case J_DILLER: {
            switch ((int)packet->skill_type)
            {
            case 0:    // 물리 공격 스킬 
                switch ((int)packet->skill_num)
                {
                case 0: { //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                    skill_cooltime(client_id, chrono::system_clock::now() + 3s, 0);
                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);
                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();


                            if ((players[i]->get_x() >= pl->get_x() - 30 && players[i]->get_x() <= pl->get_x() + 30) && (players[i]->get_z() >= pl->get_z() - 30 && players[i]->get_z() <= pl->get_z() + 30)) {
                                pl->set_skill_factor(packet->skill_type, packet->skill_num);
                                pl->phisical_skill_success(players[i], pl->get_skill_factor(packet->skill_type, packet->skill_num));

                                players[i]->state_lock.lock();
                                if (players[i]->get_state() == ST_DEAD) {
                                    players[i]->state_lock.unlock();
                                    m_SectorManager->player_remove(players[i], true, players[client_id]);
                                }
                                else players[i]->state_lock.unlock();


                                players[i]->set_target_id(pl->get_id());
                                //send_status_change_packet(pl);
                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    TimerManager::timer_queue.push(ev);

                                    //players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        Npc* bos = indun->boss;
                        if ((bos->get_x() >= pl->get_x() - 30 && bos->get_x() <= pl->get_x() + 30) && (bos->get_z() >= pl->get_z() - 30 && bos->get_z() <= pl->get_z() + 30)) {
                            pl->set_skill_factor(0, 0);
                            float give_damage = pl->get_physical_attack() * pl->get_skill_factor(0, 0);
                            float defence_damage = (bos->get_defence_factor() *
                                bos->get_physical_defence()) / (1 + (bos->get_defence_factor() *
                                    bos->get_physical_defence()));
                            float damage = give_damage * (1 - defence_damage);
                            if (bos->get_hp() > 0) {
                                bos->set_hp(bos->get_hp() - damage);
                                // physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                                //send_status_change_packet(pl);
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, damage);
                                }
                                if (bos->get_hp() <= 0) {
                                    bos->set_hp(0);
                                    indun->game_victory();
                                }
                            }
                        }
                    }
                    break;
                }
                }
                break;
            case 1:  //마법 공격 스킬  삼각형 범위?
                switch ((int)packet->skill_num)
                {
                case 0:  //물리 공격스킬 중 0번 스킬 -> 십자공격 어택 
                    skill_cooltime(client_id, chrono::system_clock::now() + 3s, 1);

                    Coord a = { pl->get_x(), pl->get_z() };    //플레이어 기준 전방 삼각형 범위 
                    Coord b = { pl->get_x() - pl->get_right_x() * 30 + pl->get_look_x() * 70,
                        pl->get_z() - pl->get_right_z() * 30 + pl->get_look_z() * 70 };  // 왼쪽 위
                    Coord c = { pl->get_x() + pl->get_right_x() * 30 + pl->get_look_x() * 70,
                        pl->get_z() + pl->get_right_z() * 30 + pl->get_look_z() * 70 };  // 오른쪽 위

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);
                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            Coord n = { players[i]->get_x(), players[i]->get_z() };

                            if (isInsideTriangle(a, b, c, n)) {
                                pl->set_skill_factor(0, 1);
                                pl->phisical_skill_success(players[i], pl->get_skill_factor(0, 1));

                                players[i]->state_lock.lock();
                                if (players[i]->get_state() == ST_DEAD) {
                                    players[i]->state_lock.unlock();
                                    m_SectorManager->player_remove(players[i], true, players[client_id]);
                                }
                                else players[i]->state_lock.unlock();

                                players[i]->set_target_id(pl->get_id());
                                //send_status_change_packet(pl);
                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    TimerManager::timer_queue.push(ev);
                                    //players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        Npc* bos = indun->boss;
                        Coord n = { bos->get_x(), bos->get_z() };

                        if (isInsideTriangle(a, b, c, n)) {
                            pl->set_skill_factor(0, 1);
                            float give_damage = pl->get_physical_attack() * pl->get_skill_factor(0, 1);
                            float defence_damage = (bos->get_defence_factor() *
                                bos->get_physical_defence()) / (1 + (bos->get_defence_factor() *
                                    bos->get_physical_defence()));
                            float damage = give_damage * (1 - defence_damage);
                            if (bos->get_hp() > 0) {
                                bos->set_hp(bos->get_hp() - damage);
                                //send_status_change_packet(pl);
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, damage);
                                }
                                if (bos->get_hp() <= 0) {
                                    bos->set_hp(0);
                                    indun->game_victory();
                                }
                            }
                        }

                    }
                    break;
                }
                break;
            case 2:  //버프   //공격력 증가로 변경 
                switch ((int)packet->skill_num)
                {
                case 0:
                    skill_cooltime(client_id, chrono::system_clock::now() + 10s, 2);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_buff_ui_packet(pl, 3); //ui
                    pl->set_physical_attack(0.6 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv()); //일단 두배 
                    pl->set_magical_attack(0.2 * pl->get_lv() * pl->get_lv() + 5 * pl->get_lv());
                    send_status_change_packet(pl);
                    break;
                }
                break;
            }
            for (int vl_id : my_vl) {
                send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_type);
            }
            send_animation_skill(pl, pl->get_id(), (int)packet->skill_type);
            break;
        }
        case J_TANKER: {
            switch ((int)packet->skill_type)
            {
            case 0:    // 물리 공격 스킬 // 방패 
                switch ((int)packet->skill_num)
                {
                case 0:  //밀어내기 
                    skill_cooltime(client_id, chrono::system_clock::now() + 3s, 0);


                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);
                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();


                            if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                pl->set_skill_factor(packet->skill_type, packet->skill_num);
                                pl->phisical_skill_success(players[i], pl->get_skill_factor(packet->skill_type, packet->skill_num));

                                players[i]->set_pos(players[i]->get_x() + pl->get_look_x() * 40, players[i]->get_z() + pl->get_look_z() * 40);

                                players[i]->state_lock.lock();
                                if (players[i]->get_state() == ST_DEAD) {
                                    players[i]->state_lock.unlock();
                                    m_SectorManager->player_remove(players[i], true, players[client_id]);
                                }
                                else {
                                    players[i]->state_lock.unlock();
                                }
                                send_move_packet(pl, players[i], 1);  //나중에 수정필요 
                                //send_status_change_packet(pl);
                                players[i]->set_target_id(pl->get_id());
                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 3s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    TimerManager::timer_queue.push(ev);
                                    players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        Npc* bos = indun->boss;
                        if ((bos->get_x() >= pl->get_x() - 15 && bos->get_x() <= pl->get_x() + 15) &&
                            (bos->get_z() >= pl->get_z() - 15 && bos->get_z() <= pl->get_z() + 15)) {
                            pl->set_skill_factor(0, 0);

                            float give_damage = pl->get_physical_attack() * pl->get_skill_factor(0, 0);
                            float defence_damage = (bos->get_defence_factor() *
                                bos->get_physical_defence()) / (1 + (bos->get_defence_factor() *
                                    bos->get_physical_defence()));
                            float damage = give_damage * (1 - defence_damage);
                            if (bos->get_hp() > 0) {
                                int m_x = 2037;
                                int m_z = 2112;
                                float r = 515.f;

                                if (sqrt(pow((bos->get_x() + pl->get_look_x() * 100 - m_x), 2) + pow((bos->get_z() + pl->get_look_z() * 100 - m_z), 2)) < r)
                                bos->set_pos(indun->boss->get_x() + pl->get_look_x() * 100, indun->boss->get_z() + pl->get_look_z() * 100);
                         
                                bos->set_hp(indun->boss->get_hp() - damage);
                                //send_move_packet(pl, dungeons[client_id]->boss, 1);  //나중에 수정필요 
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, damage);
                                }
                                send_status_change_packet(pl);
                                if (bos->get_hp() <= 0) {
                                    bos->set_hp(0);
                                    indun->game_victory();
                                }
                            }
                        }
                    }
                    break;
                }
                break;
            case 1:  //마법 공격 스킬:  어그로   다른 플레이어가 공격 도중 쓰면 대상이 나로 안바뀐다 수정 필요 
                switch ((int)packet->skill_num)
                {
                case 0:   //어그로
                    skill_cooltime(client_id, chrono::system_clock::now() + 3s, 1);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);
                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            if ((players[i]->get_x() >= pl->get_x() - 40 && players[i]->get_x() <= pl->get_x() + 40) && (players[i]->get_z() >= pl->get_z() - 40 && players[i]->get_z() <= pl->get_z() + 40)) {
                                pl->set_skill_factor((int)packet->skill_type, (int)packet->skill_num);
                                //  physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                                players[i]->set_target_id(pl->get_id());
                                //send_status_change_packet(pl);
                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    //   players[i]->set_agro_bool()
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = client_id;
                                    TimerManager::timer_queue.push(ev);
                                    //players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        if ((indun->get_x() >= pl->get_x() - 40 && indun->get_x() <= pl->get_x() + 40) &&
                            (indun->get_z() >= pl->get_z() - 40 && indun->get_z() <= pl->get_z() + 40)) {
                            pl->set_skill_factor((int)packet->skill_type, (int)packet->skill_num);
                            indun->target_id = pl->get_indun_id();
                        }
                    }
                    break;
                }
                break;
            case 2:  //버프  방어력 증가 
                switch ((int)packet->skill_num)
                {
                case 0:
                    skill_cooltime(client_id, chrono::system_clock::now() + 10s, 2);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_buff_ui_packet(pl, 1);
                    pl->set_physical_defence(0.54 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv()); //일단 두배 
                    pl->set_magical_defence(0.4 * pl->get_lv() * pl->get_lv() + 10 * pl->get_lv());
                    send_status_change_packet(pl);
                    break;
                }
                break;
            }

            for (int vl_id : my_vl) {
                send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_type);
            }
            send_animation_skill(pl, pl->get_id(), (int)packet->skill_type);
            break;
        }
        case J_SUPPORTER: { //서포터 
            switch ((int)packet->skill_type)
            {
            case 0:
                switch (packet->skill_num)
                {
                case 0: {// 사각형 내부 범위 플레이어 hp 회복  
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 0);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);

                    if (!pl->join_dungeon_room) {
                        for (int i = 0; i <= MAX_USER; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();
                            if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                players[i]->set_hp(players[i]->get_hp() + players[i]->get_maxhp() / 10);
                                send_status_change_packet(reinterpret_cast<Player*>(players[i]));
                                send_buff_ui_packet(reinterpret_cast<Player*>(players[i]), 2); //ui 
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        int tmp_hp = 0;
                        int target_player = 0;
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            if (i == 0) {
                                target_player = i;
                                tmp_hp = indun->get_party_palyer()[i]->get_hp();
                            }
                            else {
                                if (tmp_hp > indun->get_party_palyer()[i]->get_hp()) {
                                    target_player = i;
                                    tmp_hp = indun->get_party_palyer()[i]->get_hp();
                                }
                            }
                        }
                        send_status_change_packet(pl);
                        send_buff_ui_packet(indun->get_party_palyer()[target_player], 2); //ui
                        indun->get_party_palyer()[target_player]->set_hp(indun->get_party_palyer()[target_player]->get_hp() + indun->get_party_palyer()[target_player]->get_maxhp() / 10);
                        if (indun->get_party_palyer()[target_player]->get_hp() > indun->get_party_palyer()[target_player]->get_maxhp())
                            indun->get_party_palyer()[target_player]->set_hp(indun->get_party_palyer()[target_player]->get_maxhp());

                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            send_change_hp_packet(indun->get_party_palyer()[i], indun->get_party_palyer()[target_player], 0);
                        }
                    }
                }
                      break;
                }
                break;
            case 1: //버프
                switch (packet->skill_num)
                {
                case 0: {  //mp회복 
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 1);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);

                    if (!pl->join_dungeon_room) {
                        for (int i = 0; i <= MAX_USER; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();
                            if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                players[i]->set_mp(players[i]->get_mp() + players[i]->get_maxmp() / 10);
                                send_change_mp_packet(pl, players[i]);
                                send_buff_ui_packet(reinterpret_cast<Player*>(players[i]), 0); //ui 

                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        int tmp_mp = 0;
                        int target_player = 0;
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            if (i == 0) {
                                target_player = i;
                                tmp_mp = indun->get_party_palyer()[i]->get_mp();
                            }
                            else {
                                if (tmp_mp > indun->get_party_palyer()[i]->get_mp()) {
                                    target_player = i;
                                    tmp_mp = indun->get_party_palyer()[i]->get_mp();
                                }
                            }
                        }
                        send_status_change_packet(pl);
                        send_buff_ui_packet(indun->get_party_palyer()[target_player], 0); //ui
                        indun->get_party_palyer()[target_player]->set_mp(indun->get_party_palyer()[target_player]->get_mp() + indun->get_party_palyer()[target_player]->get_maxmp() / 10);
                        if (indun->get_party_palyer()[target_player]->get_mp() > indun->get_party_palyer()[target_player]->get_maxmp())
                            indun->get_party_palyer()[target_player]->set_mp(indun->get_party_palyer()[target_player]->get_maxmp());
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            send_change_mp_packet(indun->get_party_palyer()[i], indun->get_party_palyer()[target_player]);
                        }
                    }

                }
                      break;
                }
                break;
            case 2:
                switch (packet->skill_num)
                {
                case 0: {//공속 
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 2);

                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);

                    if (!pl->join_dungeon_room) {
                        for (int i = 0; i <= MAX_USER; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();
                            if ((players[i]->get_x() >= pl->get_x() - 15 && players[i]->get_x() <= pl->get_x() + 15) && (players[i]->get_z() >= pl->get_z() - 15 && players[i]->get_z() <= pl->get_z() + 15)) {
                                reinterpret_cast<Player*>(players[i])->attack_speed_up = 1;
                                send_buff_ui_packet(reinterpret_cast<Player*>(players[i]), 4);
                            }
                        }
                    }
                    else {
                        Gaia* indun = m_ObjectManger->get_dungeon(pl->get_indun_id());
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            indun->get_party_palyer()[i]->attack_speed_up = 1;
                            send_buff_ui_packet(indun->get_party_palyer()[i], 4);
                        }
                    }
                }
                      break;
                }
                break;
            }
            if (packet->skill_type == 0) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num);
            }
            else if (packet->skill_type == 1) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num + 1);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num + 1);
            }
            else if (packet->skill_type == 2) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num + 2);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num + 2);
            }
            break;
        }
        case J_MAGICIAN: {
            switch ((int)packet->skill_type)
            {
            case 0:
                switch ((int)packet->skill_num) {
                case 0:// hp희생해 상대 hp를 mp로 흡수 
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 0);

                    pl->set_hp(pl->get_hp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);

                    if (!pl->join_dungeon_room) {

                    for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                        players[i]->state_lock.lock();
                        if (players[i]->get_state() != ST_INGAME) {
                            players[i]->state_lock.unlock();
                            continue;
                        }
                        players[i]->state_lock.unlock();

                        if ((players[i]->get_x() >= pl->get_x() - 30 && players[i]->get_x() <= pl->get_x() + 30) && (players[i]->get_z() >= pl->get_z() - 30 && players[i]->get_z() <= pl->get_z() + 30)) {

                            pl->set_mp(pl->get_mp() + players[i]->get_hp() / 10);
                            if (pl->get_mp() > pl->get_maxmp())
                                pl->set_mp(pl->get_maxmp());

                            pl->set_skill_factor(1, 0);
                            pl->magical_skill_success(players[i], pl->get_skill_factor(1, 0));

                            players[i]->state_lock.lock();
                            if (players[i]->get_state() == ST_DEAD) {
                                players[i]->state_lock.unlock();
                                m_SectorManager->player_remove(players[i], true, players[client_id]);
                            }
                            else players[i]->state_lock.unlock();

                            players[i]->set_target_id(pl->get_id());
                            send_status_change_packet(pl);
                            send_status_change_packet(reinterpret_cast<Player*>(players[i]));
                            if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                players[i]->set_active(true);
                                timer_event ev;
                                ev.obj_id = i;
                                ev.start_time = chrono::system_clock::now() + 1s;
                                ev.ev = EVENT_NPC_ATTACK;
                                ev.target_id = players[i]->get_target_id();
                                TimerManager::timer_queue.push(ev);
                            }
                        }
                    }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        if ((indun->get_x() >= pl->get_x() - 30 && indun->get_x() <= pl->get_x() + 30) &&
                            (indun->get_z() >= pl->get_z() - 30 && indun->get_z() <= pl->get_z() + 30)) {
                            pl->set_skill_factor(1, 0);
                            indun->target_id = pl->get_indun_id();
                            Npc* bos = indun->boss;
                            pl->magical_skill_success(bos, pl->get_skill_factor(1, 0));
                            for (int i = 0; i < GAIA_ROOM; ++i) {
                                send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, 0);
                            }
                            if (bos->get_hp() <= 0) {
                                bos->set_hp(0);
                                indun->game_victory();
                            }
                        }
                    }
                    break;
                }
                break;
            case 1:
                switch ((int)packet->skill_num)
                {
                case 0: {
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 1);
                //    send_play_shoot_packet(pl);
                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);
                    //좌우 삼각형 두개로 사각형 범위 ?
                    Coord a = { pl->get_x() + pl->get_right_x() * -5, pl->get_z() + pl->get_right_z() * -5 };
                    Coord b = { pl->get_x() + pl->get_right_x() * 5, pl->get_z() + pl->get_right_z() * 5 };
                    Coord c = { (pl->get_x() + pl->get_right_x() * -5) + pl->get_look_x() * 140,
                   (pl->get_z() + pl->get_right_z() * -5) + pl->get_look_z() * 140, };


                    Coord d = { pl->get_x() + pl->get_right_x() * 5, pl->get_z() + pl->get_right_z() * 5 };
                    Coord e = { (pl->get_x() + pl->get_right_x() * 5) + pl->get_look_x() * 140
                        , (pl->get_z() + pl->get_right_z() * 5) + pl->get_look_x() * 140 };
                    Coord f = { (pl->get_x() + pl->get_right_x() * -5) + pl->get_look_x() * 140,
                   (pl->get_z() + pl->get_right_z() * -5) + pl->get_look_z() * 140, };

                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            Coord n = { players[i]->get_x(), players[i]->get_z() };

                            if (isInsideTriangle(a, b, c, n) || isInsideTriangle(d, e, f, n)) {
                                pl->set_skill_factor(1, 1);
                                players[i]->set_target_id(pl->get_id());
                                pl->magical_skill_success(players[i], pl->get_skill_factor(1, 1));
                                send_play_effect_packet(pl, players[i]); // 이펙트 터트릴 위치 

                                players[i]->state_lock.lock();
                                if (players[i]->get_state() == ST_DEAD) {
                                    players[i]->state_lock.unlock();
                                    m_SectorManager->player_remove(players[i], true, players[client_id]);
                                }
                                else players[i]->state_lock.unlock();

                                players[i]->set_target_id(pl->get_id());

                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    TimerManager::timer_queue.push(ev);
                                    //players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else{
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id); 
                        Npc* bos = indun->boss;
                        Coord gaia_pos = { bos->get_x(),bos->get_z() };
                        if (isInsideTriangle(a, b, c, gaia_pos) || isInsideTriangle(d, e, f, gaia_pos)) {
                            pl->set_skill_factor(1, 1);
                            pl->magical_skill_success(bos, pl->get_skill_factor(1, 1));
                            send_play_effect_packet(pl, bos); // 이펙트 터트릴 위치 

                            for (int i = 0; i < GAIA_ROOM; ++i) {
                                send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, 0);
                            }
                            if (bos->get_hp() <= 0) {
                                bos->set_hp(0);
                                indun->game_victory();
                            }
                        }

                    }
                }
                      break;
                }
                break;
            case 2:// 빔 
                switch ((int)packet->skill_num)
                {
                case 0:
                    skill_cooltime(client_id, chrono::system_clock::now() + 5s, 2);
                    // send_play_shoot_packet(pl); 다른 걸로 
                    pl->set_mp(pl->get_mp() - pl->get_lv() * 10);
                    send_status_change_packet(pl);

                    Coord a1 = { pl->get_x() + pl->get_right_x() * -10, pl->get_z() + pl->get_right_z() * -10 };
                    Coord b1 = { pl->get_x() + pl->get_right_x() * 10, pl->get_z() + pl->get_right_z() * 10 };
                    Coord c1 = { (pl->get_x() + pl->get_right_x() * -10) + pl->get_look_x() * 140,
                   (pl->get_z() + pl->get_right_z() * -10) + pl->get_look_z() * 140, };


                    Coord d1 = { pl->get_x() + pl->get_right_x() * 10, pl->get_z() + pl->get_right_z() * 10 };
                    Coord e1 = { (pl->get_x() + pl->get_right_x() * 10) + pl->get_look_x() * 140
                        , (pl->get_z() + pl->get_right_z() * 10) + pl->get_look_x() * 140 };
                    Coord f1 = { (pl->get_x() + pl->get_right_x() * -10) + pl->get_look_x() * 140,
                   (pl->get_z() + pl->get_right_z() * -10) + pl->get_look_z() * 140, };

                    if (!pl->join_dungeon_room) {
                        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                            players[i]->state_lock.lock();
                            if (players[i]->get_state() != ST_INGAME) {
                                players[i]->state_lock.unlock();
                                continue;
                            }
                            players[i]->state_lock.unlock();

                            Coord n = { players[i]->get_x(), players[i]->get_z() };

                            if (isInsideTriangle(a1, b1, c1, n) || isInsideTriangle(d1, e1, f1, n)) {

                                pl->set_skill_factor(1, 2);
                                players[i]->set_target_id(pl->get_id());
                                pl->magical_skill_success(players[i], pl->get_skill_factor(1, 2));
                                send_play_effect_packet(pl, players[i]); // 이펙트 터트릴 위치 

                                players[i]->state_lock.lock();
                                if (players[i]->get_state() == ST_DEAD) {
                                    players[i]->state_lock.unlock();
                                    m_SectorManager->player_remove(players[i], true, players[client_id]);
                                }
                                else players[i]->state_lock.unlock();

                                players[i]->set_target_id(pl->get_id());

                                if (players[i]->get_active() == false && players[i]->get_tribe() == MONSTER) {
                                    players[i]->set_active(true);
                                    timer_event ev;
                                    ev.obj_id = i;
                                    ev.start_time = chrono::system_clock::now() + 1s;
                                    ev.ev = EVENT_NPC_ATTACK;
                                    ev.target_id = players[i]->get_target_id();
                                    TimerManager::timer_queue.push(ev);
                                    //players[i]->push_npc_move_event();
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                        Npc* bos = indun->boss;
                        Coord gaia_pos = { bos->get_x(),bos->get_z() };
                        if (isInsideTriangle(a1, b1, c1, gaia_pos) || isInsideTriangle(d1, e1, f1, gaia_pos)) {
                            pl->set_skill_factor(1, 2);
                            pl->magical_skill_success(bos, pl->get_skill_factor(1, 2));
                            send_play_effect_packet(pl, bos); // 이펙트 터트릴 위치 

                            for (int i = 0; i < GAIA_ROOM; ++i) {
                                send_change_hp_packet(indun->get_party_palyer()[i], indun->boss, 0);
                            }
                            if (bos->get_hp() <= 0) {
                                bos->set_hp(0);
                                indun->game_victory();
                            }
                        }

                    }
                    break;
                }
                break;
            }
            if (packet->skill_type == 0) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num);
            }
            else if (packet->skill_type == 1) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num + 1);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num + 1);
            }
            else if (packet->skill_type == 2) {
                for (int vl_id : my_vl) {
                    send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num + 2);
                }
                send_animation_skill(pl, pl->get_id(), (int)packet->skill_num + 2);
            }
            break;
        }
            break;
        }
        break;
        

        break;
    }
    case CS_PACKET_LOOK: {
        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        cs_packet_look* packet = reinterpret_cast<cs_packet_look*>(p);
        pl->set_look(packet->x, packet->y, packet->z);
        pl->set_right(packet->right_x, packet->right_y, packet->right_z);

        // 근처에 있는 모든 플레이어에게 방향이 바뀌었다는것을 보내준다
        pl->vl.lock();
        unordered_set <int> my_vl{ pl->viewlist };
        pl->vl.unlock();

        for (auto i : my_vl) {
            // Npc
            if (m_ObjectManger->is_npc(i) == true) continue;
            // Player
            send_look_packet(reinterpret_cast<Player*>(m_ObjectManger->get_player(i)), pl);
        }
        break;
    }
    case CS_PACKET_CHANGE_JOB: {
        cs_packet_change_job* packet = reinterpret_cast<cs_packet_change_job*>(p);
        pl->set_job(packet->job);

        switch (pl->get_job()) {
        case J_DILLER: {
            Diller::Initialize(pl);
            break;
        }
        case J_TANKER: {
            Tanker::Initialize(pl);
            break;
        }
        case J_SUPPORTER: {
            Supporter::Initialize(pl);
            break;
        }
        case J_MAGICIAN: {
            Magician::Initialize(pl);
            break;
        }
        }
        send_status_change_packet(pl);
        break;
    }
    case CS_PACKET_CHANGE_ELEMENT: {
        cs_packet_change_element* packet = reinterpret_cast<cs_packet_change_element*>(p);
        pl->set_element(packet->element);
        send_status_change_packet(pl);
        break;
    }
    case CS_PACKET_PICKING_SKILL: {
        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        cs_packet_picking_skill* packet = reinterpret_cast<cs_packet_picking_skill*>(p);
        if (pl->get_skill_active(packet->skill_type) == true) return;
        pl->set_skill_active(packet->skill_type, true);

        switch (packet->skill_type)
        {
        case 0:
            switch (packet->skill_num)
            {
            case 0:
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                ev.ev = EVENT_SKILL_COOLTIME;
                ev.target_id = 1; // packet->target;
                TimerManager::timer_queue.push(ev);


                pl->set_mp(pl->get_mp() - 1000);
                send_status_change_packet(pl);

                int taget = packet->target;// -9615;



                if (!pl->join_dungeon_room) {
                }

                else {
                    int indun_id = pl->get_indun_id();
                    players[taget]->set_mp(players[taget]->get_mp() + players[taget]->get_maxmp() / 10);
                    Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                    for (int i = 0; i < GAIA_ROOM; ++i) {
                        send_change_mp_packet(indun->get_party_palyer()[i], players[taget]);
                    }
                }
                

                send_buff_ui_packet(reinterpret_cast<Player*>(players[taget]), 0);
                break;

            }
            break;
        case 1:
            switch (packet->skill_num)
            {
            case 0:
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                ev.ev = EVENT_SKILL_COOLTIME;
                ev.target_id = 2; //packet->target;
                TimerManager::timer_queue.push(ev);

                pl->set_mp(pl->get_mp() - 1000);
                send_status_change_packet(pl);

                int taget = packet->target; //  -9615;

                players[taget]->set_physical_defence(players[taget]->get_physical_defence() * 11 / 10);
                players[taget]->set_magical_defence(players[taget]->get_magical_defence() * 11 / 10);
                send_status_change_packet(reinterpret_cast<Player*>(players[taget]));
                send_buff_ui_packet(reinterpret_cast<Player*>(players[taget]), 1);
                break;

            }
            break;
        case 2:
            switch (packet->skill_num)
            {
            case 0:
                timer_event ev;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                ev.ev = EVENT_SKILL_COOLTIME;
                ev.target_id = 0;// packet->target;
                TimerManager::timer_queue.push(ev);

                pl->set_mp(pl->get_mp() - 1000);
                send_status_change_packet(pl);

                int taget = packet->target;// - 9615;
                if (!pl->join_dungeon_room) {
                }

                else {
                    int indun_id = pl->get_indun_id();
                    players[taget]->set_hp(players[taget]->get_hp() + players[taget]->get_maxhp() / 10);
                    Gaia* indun = m_ObjectManger->get_dungeon(indun_id);
                    for (int i = 0; i < GAIA_ROOM; ++i) {
                        send_change_hp_packet(indun->get_party_palyer()[i], players[taget], 0);
                    }
                }
                send_buff_ui_packet(reinterpret_cast<Player*>(players[taget]), 2);
                break;

            }

            break;
        }
        break;
    }
    case CS_PACKET_PARTY_ROOM: {
        // 현재 활성화 되었는 던전의 정보들을 보낸다
        for (auto& dun : m_ObjectManger->get_dungeons()) {
            dun->state_lock.lock();
            if (dun->get_dun_st() == DUN_ST_ROBBY) {
                // 던전의 정보들을 보내준다
                dun->state_lock.unlock();
                send_party_room_packet(pl, dun->get_party_name(), dun->get_dungeon_id());
                continue;
            }
            dun->state_lock.unlock();
        }
        break;
    }
    case CS_PACKET_PARTY_ROOM_MAKE: {
        pl->state_lock.lock();
        if (pl->get_state() != ST_INGAME || pl->join_dungeon_room == true) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        for (auto& dun : m_ObjectManger->get_dungeons()) {
            // join dungeon party
            dun->state_lock.lock();
            if (dun->get_dun_st() == DUN_ST_FREE) {
                dun->set_dun_st(DUN_ST_ROBBY);
                dun->state_lock.unlock();

                // 이 방에 이 플레이어를 집어 넣는다
                dun->set_party_name(pl->get_name());
                dun->join_player(pl);

                // 이 방에 대한 정보를 보내준다
                send_party_room_packet(pl, dun->get_party_name(), dun->get_dungeon_id());
                send_party_room_info_packet(pl, dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
                send_party_room_enter_ok_packet(pl, dun->get_dungeon_id());
                break;
            }
            dun->state_lock.unlock();
        }
        break;
    }
    case CS_PACKET_PARTY_ROOM_INFO_REQUEST: {
        int r_id = reinterpret_cast<cs_packet_party_room_info_request*>(p)->room_id;
        Gaia* dun = m_ObjectManger->get_dungeon(r_id);
        send_party_room_info_packet(pl, dun->get_party_palyer(),
            dun->player_cnt, dun->get_dungeon_id());
        break;
    }
    case CS_PACKET_RAID_RANDER_OK: {
        Gaia* dun = m_ObjectManger->get_dungeon(pl->indun_id);
        dun->player_rander_ok++;
        if (dun->player_rander_ok == GAIA_ROOM - dun->partner_cnt) {
            dun->start_game = true;

            // Ai움직이기 시작
            Player** party_players = dun->get_party_palyer();

            // 레이드 위치 수정
            for (int i = 0; i < GAIA_ROOM; i++) {
                party_players[i]->set_x(2025 + 10 * i);
                party_players[i]->set_z(2110);
                for (int j = 0; j < GAIA_ROOM; j++) {
                    send_move_packet(party_players[j], party_players[i], 0);
                }
            }


            // BOSS NPC Timer Start
            timer_event ev;
            ev.obj_id = dun->get_dungeon_id();
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_BOSS_MOVE;
            ev.target_id = -1;
            TimerManager::timer_queue.push(ev);

            ZeroMemory(&ev, sizeof(ev));
            ev.obj_id = dun->get_dungeon_id();
            ev.start_time = chrono::system_clock::now() + 3s;
            ev.ev = EVENT_BOSS_ATTACK;
            ev.target_id = -1;
            TimerManager::timer_queue.push(ev);

            for (int i = 0; i < GAIA_ROOM; i++) {
                if (party_players[i]->get_tribe() == PARTNER) {
                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_PARTNER_MOVE;
                    ev.target_id = 1;
                    TimerManager::timer_queue.push(ev);

                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_PARTNER_NORMAL_ATTACK;
                    ev.target_id = 1;
                    TimerManager::timer_queue.push(ev);

                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 3s;
                    ev.ev = EVENT_PARTNER_SKILL;
                    ev.target_id = 1;
                    TimerManager::timer_queue.push(ev);
                }
            }

        }
        break;
    }
    case CS_PACKET_PARTY_ROOM_ENTER_REQUEST: {
        int r_id = (int)reinterpret_cast<cs_packet_party_room_enter_request*>(p)->room_id;
        pl->state_lock.lock();
        if (pl->get_state() != ST_INGAME || pl->join_dungeon_room == true) {
            if (pl->get_state() == ST_INGAME) send_party_room_enter_failed_packet(pl, r_id, 2);
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();

        Gaia* dun = m_ObjectManger->get_dungeon(r_id);

        // join dungeon party
        dun->state_lock.lock();
        if (dun->get_dun_st() != DUN_ST_ROBBY) {
            // 던전입장 실패 패킷 보내기
            if (dun->get_dun_st() == DUN_ST_FREE) send_party_room_enter_failed_packet(pl, r_id, 1);
            else send_party_room_enter_failed_packet(pl, r_id, 0);
            dun->state_lock.unlock();
            break;
        }
        dun->state_lock.unlock();
        // 이 방에 이 플레이어를 집어 넣는다
        if (dun->player_cnt == GAIA_ROOM) {
            send_party_room_enter_failed_packet(pl, r_id, 0);
            break;
        }
        dun->join_player(pl);

        // 이 방에 대한 정보를 보내준다
        for (auto& dun : m_ObjectManger->get_dungeons()) {
            dun->state_lock.lock();
            if (dun->get_dun_st() == DUN_ST_ROBBY) {
                // 던전의 정보들을 보내준다
                dun->state_lock.unlock();
                send_party_room_packet(pl, dun->get_party_name(), dun->get_dungeon_id());
                continue;
            }
            dun->state_lock.unlock();
        }

        Player** party_players = dun->get_party_palyer();
        for (int i = 0; i < dun->player_cnt; i++) {
            send_party_room_info_packet(party_players[i], dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
        }
        send_party_room_enter_ok_packet(pl, dun->get_dungeon_id());
        break;
    }
    case CS_PACKET_PARTY_ROOM_QUIT_REQUEST: {
        int r_id = (int)reinterpret_cast<cs_packet_party_room_quit_request*>(p)->room_id;
        Gaia* dun = m_ObjectManger->get_dungeon(r_id);
        dun->quit_palyer(pl);
        // 나갔다는 정보를 player에게 보내준다
        send_party_room_quit_ok_packet(pl);
        pl->join_dungeon_room = false;

        Player** party_players = dun->get_party_palyer();
        if (dun->player_cnt - dun->partner_cnt == 0) {
            // 아무도 없다는 뜻
            for (auto& pls : players) {
                if (true == m_ObjectManger->is_npc(pls->get_id())) break;
                pls->state_lock.lock();
                if (ST_INGAME != pls->get_state()) {
                    pls->state_lock.unlock();
                    continue;
                }
                pls->state_lock.unlock();
                send_party_room_destroy(reinterpret_cast<Player*>(pls), r_id);
            }

            for (int i = 0; i < dun->player_cnt; i++) {
                int delete_id = party_players[i]->get_id();
                players[delete_id]->state_lock.lock();
                players[delete_id]->set_state(ST_FREE);
                players[delete_id]->state_lock.unlock();
            }
            dun->destroy_dungeon();

        }
        else {
            for (int i = 0; i < dun->player_cnt; i++) {
                send_party_room_info_packet(party_players[i], dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
            }
        }

        break;
    }
    case CS_PACKET_PARTY_INVITE: {
        cs_packet_party_invite* packet = reinterpret_cast<cs_packet_party_invite*>(p);
        bool find_player = false;
        for (auto& check_pl : players) {
            check_pl->state_lock.lock();
            if (check_pl->get_state() != ST_INGAME) {
                check_pl->state_lock.unlock();
                continue;
            }
            else {
                if (check_pl->get_tribe() != HUMAN) {
                    check_pl->state_lock.unlock();
                    break;
                }
                check_pl->state_lock.unlock();
                // 이름 비교
                char* tmp = packet->user_name;
                if (strcmp(check_pl->get_name(), packet->user_name) == 0) {
                    find_player = true;
                    if (reinterpret_cast<Player*>(check_pl)->join_dungeon_room == true) {
                        send_party_invitation_failed(pl, 2, packet->user_name);
                    }
                    else send_party_invitation(reinterpret_cast<Player*>(check_pl), (int)reinterpret_cast<cs_packet_party_invite*>(p)->room_id, pl->get_id());
                    break;
                }
                else continue;
            }
        }
        if (find_player == false) send_party_invitation_failed(pl, 0, packet->user_name);

        break;
    }
    case CS_PACKET_PARTY_INVITATION_REPLY: {
        cs_packet_party_invitation_reply* packet = reinterpret_cast<cs_packet_party_invitation_reply*>(p);
        int r_id = (int)packet->room_id;
        if ((int)packet->accept == 1) {
            Gaia* dun = m_ObjectManger->get_dungeon(r_id);

            // join dungeon party
            dun->state_lock.lock();
            if (dun->get_dun_st() != DUN_ST_ROBBY) {
                // 던전입장 실패 패킷 보내기
                if (dun->get_dun_st() == DUN_ST_FREE) send_party_room_enter_failed_packet(pl, r_id, 1);
                else send_party_room_enter_failed_packet(pl, r_id, 0);
                dun->state_lock.unlock();
                break;
            }
            dun->state_lock.unlock();
            // 이 방에 이 플레이어를 집어 넣는다
            if (dun->player_cnt == GAIA_ROOM) {
                send_party_room_enter_failed_packet(pl, r_id, 0);
                break;
            }
            dun->join_player(pl);

            // 이 방에 대한 정보를 보내준다
            for (auto& duns : m_ObjectManger->get_dungeons()) {
                duns->state_lock.lock();
                if (duns->get_dun_st() == DUN_ST_ROBBY) {
                    // 던전의 정보들을 보내준다
                    duns->state_lock.unlock();
                    send_party_room_packet(pl, duns->get_party_name(), duns->get_dungeon_id());
                    continue;
                }
                duns->state_lock.unlock();
            }

            Player** party_players = dun->get_party_palyer();

            for (int i = 0; i < dun->player_cnt; i++) {
                send_party_room_info_packet(party_players[i], dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
            }
            send_party_room_enter_ok_packet(pl, dun->get_dungeon_id());
        }
        else if ((int)packet->accept == 0) {
            send_party_invitation_failed(reinterpret_cast<Player*>(players[packet->invite_user_id]), 1, pl->get_name());
        }
        break;
    }
    case CS_PACKET_PARTY_ADD_PARTNER: {
        cs_packet_party_add_partner* packet = reinterpret_cast<cs_packet_party_add_partner*>(p);
        int r_id = (int)reinterpret_cast<cs_packet_party_add_partner*>(p)->room_id;
        Gaia* dun = m_ObjectManger->get_dungeon(r_id);

        if (dun->player_cnt < GAIA_ROOM) {  // 제한 인원수 보다 적을 때만 추가 가능하도록 하자 

            int new_id = m_ObjectManger->get_new_ai_id();
            if (-1 == new_id) {
                cout << "Maxmum user overflow.Accept aborted.\n";
            }
            players[new_id]->state_lock.lock();
            players[new_id]->set_state(ST_ACCEPT);
            players[new_id]->state_lock.unlock();

            Partner* partner = reinterpret_cast<Partner*>(players[new_id]);

            // players 에서 파트너의 아이디와 기본정보 업데이트 
            char ai_name[MAX_NAME_SIZE];
            sprintf_s(ai_name, "%s%d", "AI", dun->partner_cnt);


            partner->set_tribe(PARTNER);
            partner->set_id(new_id);
            partner->set_name(ai_name);
            partner->set_x(2100);
            partner->set_y(0);
            partner->set_z(1940);

            partner->set_job(static_cast<JOB>(packet->job));
            partner->set_lv( players[client_id]->get_lv());

            switch (partner->get_job()) {
            case J_DILLER: {
                Diller::Initialize(partner);
                break;
            }
            case J_TANKER: {
                Tanker::Initialize(partner);
                break;
            }
            case J_MAGICIAN: {
                Magician::Initialize(partner);
                break;
            }
            case J_SUPPORTER: {
                Supporter::Initialize(partner);
                break;
            }
            }

            /*partner->set_maxhp(10000);
            partner->set_maxmp(10000);
            partner->set_hp(1000);
            partner->set_mp(partner->get_maxmp());
            partner->set_physical_attack(0.3 * 25 * 25 + 10 * 25);
            partner->set_magical_attack(0.1 * 25 * 25 + 5 * 25);
            partner->set_physical_defence(0.24 * 25 * 25 + 10 * 25);
            partner->set_magical_defence(0.17 * 25 * 25 + 10 * 25);
            partner->set_basic_attack_factor(50.0f);
            partner->set_defence_factor(0.0002);*/
            
            partner->set_element(players[client_id]->get_element());

            partner->set_origin_physical_attack(partner->get_physical_attack());
            partner->set_origin_magical_attack(partner->get_magical_attack());
            partner->set_origin_physical_defence(partner->get_physical_defence());
            partner->set_origin_magical_defence(partner->get_magical_defence());

            //  여기까지 클라에서 패킷 받으면, 새 player id 생성 후 정보 초기화  

            // join dungeon party
            // 이 방에 이 플레이어를 집어 넣는다
            dun->partner_cnt++;
            dun->join_player(reinterpret_cast<Player*>(players[new_id]));
           // dun->player_cnt

            // 이 방에 대한 정보를 보내준다
            Player** party_players = dun->get_party_palyer();     
            for (int i = 0; i < dun->player_cnt; i++) {
                if (party_players[i]->get_tribe() == HUMAN)
                    send_party_room_info_packet(party_players[i], dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
            }
        }
        else
            break;

        break;
    }
    case CS_PACKET_RE_LOGIN: {
        cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
        // 중복 아이디 검사
        for (auto* p : players) {
            if (p->get_tribe() != HUMAN) break;
            if (p->get_state() == ST_FREE) continue;
            if (p->get_id() == client_id) continue;
            if (strcmp(packet->id, "admin") == 0) break;

            if (strcmp(reinterpret_cast<Player*>(p)->get_login_id(), packet->id) == 0) {
                cout << "중복된 아이디 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
               // m_ObjectManger->Disconnect(client_id);
                return;
            }
            if (strcmp(reinterpret_cast<Player*>(p)->get_name(), packet->name) == 0) {
                cout << "중복된 닉네임 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
               // m_ObjectManger->Disconnect(client_id);
                return;
            }

        }
        pl->set_login_id(packet->id);
        //데이터 베이스 
        bool login = false;
        login = Add_DB(packet->id, packet->password, pl, packet->nickname, (int)packet->job, (int)packet->element);
        
        
        if (login == false) {
            //login = true; DB연결X 시 주석 빼자 
            if (DB_On == true) {
                send_login_fail_packet(pl, 2);
            }
        }

        // 원래는 DB에서 받아와야 하는 정보를 기본 정보로 대체
        if (DB_On == false) {
            pl->set_login_id(packet->id);
            pl->set_name(packet->nickname);
            pl->set_x(3200);
            pl->set_y(32);
            pl->set_z(785);
            pl->set_lv(25);
            pl->set_job((JOB)packet->job);
            pl->set_element((ELEMENT)packet->element);
            /*
            pl->set_x(3210);
            pl->set_y(0);
            pl->set_z(940);
            pl->set_job(static_cast<JOB>(packet->job));
            //pl->set_job(J_DILLER);
            pl->set_lv(25);
            pl->set_element(E_WATER);
            pl->set_exp(1000);
            pl->set_name(packet->name);
            pl->set_login_id(packet->id);

            pl->indun_id - 1;
            pl->join_dungeon_room = false;*/
        }

        // Stress Test용
        if (strcmp(packet->id, "admin") == 0) {
            pl->set_x(rand() % 4000);
            pl->set_z(rand() % 4000);
        }

        switch (pl->get_job()) {
        case J_DILLER: {
            Diller::Initialize(pl);
            break;
        }
        case J_TANKER: {
            Tanker::Initialize(pl);
            break;
        }
        case J_SUPPORTER: {
            Supporter::Initialize(pl);
            break;
        }
        case J_MAGICIAN: {
            Magician::Initialize(pl);
            break;
        }
        default: {
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
                TimerManager::timer_queue.push(ev);
                reinterpret_cast<Player*>(players[client_id])->_auto_hp = true;
            }
        }
        else pl->_auto_hp = false;

        if (login == true)
            send_login_ok_packet(pl);
        else if (DB_On == false && login == false) {
            send_login_ok_packet(pl);
        }
        pl->state_lock.lock();
        pl->set_state(ST_INGAME);
        pl->state_lock.unlock();

        m_SectorManager->player_put(pl);
        Save_position(pl);
        break;
    }
    default:
        break;
    }
}

void PacketManager::skill_cooltime(int client_id, chrono::system_clock::time_point t, int skill_id)
{
    timer_event ev;
    ev.obj_id = client_id;
    ev.start_time = t;  //쿨타임
    ev.ev = EVENT_SKILL_COOLTIME;
    ev.target_id = skill_id;
    TimerManager::timer_queue.push(ev);
}


