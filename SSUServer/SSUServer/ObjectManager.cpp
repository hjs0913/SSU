#include "ObjectManager.h"
#include "send.h"
#include "PacketManager.h"
#include "TimerManager.h"
#include "database.h"
#include <fstream>

ObjectManager* static_ObjectManager::objManager = nullptr;

ObjectManager::ObjectManager(SectorManager* sectorManager, MainSocketManager* socket)
{
    m_SectorManager = sectorManager;
    s_socket = socket->get_socket();
    h_iocp =  socket->get_iocp();

    static_ObjectManager::set_objManger(this);

    for (int i = 0; i < MAX_USER; ++i) {
        players[i] = new Player(i);
    }
    cout << "플레이어 초기화 완료" << endl;
    Initialize_Npc();

    Initialize_Ai();

    Initialize_Obstacle();

    Initialize_Dungeons();

    m_SectorManager->set_players_object(players);

    for (int i = NPC_ID_START; i < NPC_ID_END; i++) {
        m_SectorManager->player_put(players[i]);
    }
}

void ObjectManager::Initialize_Npc()
{
    const int interval = 30;
    int npc_num = 0;

    for (int i = NPC_ID_START + interval*npc_num; i < NPC_ID_START + interval * (npc_num+1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("fallen_rabbit.lua");
    }
    npc_num++;
    for (int i = NPC_ID_START + interval * npc_num; i < NPC_ID_START + interval * (npc_num + 1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("fallen_spider.lua");
    }
    npc_num++;
    for (int i = NPC_ID_START + interval * npc_num; i < NPC_ID_START + interval * (npc_num + 1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("fallen_frog.lua");
    }
    npc_num++;
    for (int i = NPC_ID_START + interval * npc_num; i < NPC_ID_START + interval * (npc_num + 1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("fallen_monkey.lua");
    }
    npc_num++;
    for (int i = NPC_ID_START + interval * npc_num; i < NPC_ID_START + interval * (npc_num + 1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("wolf_boss.lua");
        players[i]->set_tribe(AGRO);
    }
    npc_num++;
    for (int i = NPC_ID_START + interval * npc_num; i < NPC_ID_START + interval * (npc_num + 1); i++) {
        players[i] = new Npc(i);
        players[i]->Initialize_Lua("fallen_pig.lua");
        players[i]->set_tribe(AGRO);
    }
    npc_num++;
    cout << "NPC 초기화 완료" << endl;
}

void ObjectManager::Initialize_Obstacle()
{
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        obstacles[i] = new Obstacle(i);
    }

    ifstream obstacles_read("tree_position.txt");
    if (!obstacles_read.is_open()) {
        cout << "파일을 읽을 수 없습니다" << endl;
        exit(0);
    }

    for (int i = 0; i < 609; i++) {
        float x, y, z;
        obstacles_read >> x >> y >> z;
        obstacles[i]->set_id(i);
        obstacles[i]->set_x(x);
        obstacles[i]->set_y(y);
        obstacles[i]->set_z(z);
    }
    obstacles_read.close();

    fstream house_read("house_position.txt");
    if (!house_read.is_open()) {
        cout << "파일을 읽을 수 없습니다" << endl;
        exit(0);
    }

    for (int i = 0; i < 48; i++) {
        house_obstacles[i] = new Obstacle(i/4);
    }

    for (int i = 0; i < 48; i++) {
        float x, z;
        house_read >> x >> z;
        house_obstacles[i]->set_id(i/4);
        house_obstacles[i]->set_x(x);
        house_obstacles[i]->set_y(0);
        house_obstacles[i]->set_z(z);
    }
    house_read.close();


    cout << "장애물 초기화 완료" << endl;
}

void ObjectManager::Initialize_Dungeons()
{
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        dungeons[i] = new Gaia(i);
    }
    cout << "던전 초기화 완료" << endl;
}

void ObjectManager::Initialize_Ai()
{
    for (int i = AI_ID_START; i < AI_ID_END; ++i) {
        players[i] = new Partner(i);
    }
}

void ObjectManager::error_display(int err_no)
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

void ObjectManager::Disconnect(int c_id)
{
    Player* pl = reinterpret_cast<Player*>(players[c_id]);
    // indun처리
    // viewlist 처리
    pl->vl.lock();
    unordered_set <int> my_vl = pl->viewlist;
    if (pl->join_dungeon_room == true) {
        pl->join_dungeon_room == false;
        pl->vl.unlock();
        Gaia* dun = dungeons[pl->get_indun_id()];
        dun->quit_palyer(pl);

        Player** party_players = dun->get_party_palyer();
        if (dun->player_cnt - dun->partner_cnt == 0) {
            // 아무도 없다는 뜻
            dun->state_lock.lock();
            dun->set_dun_st(DUN_ST_FREE);
            dun->state_lock.unlock();
            for (auto& pls : players) {
                if (true == is_npc(pls->get_id())) break;
                pls->state_lock.lock();
                if (ST_INGAME != pls->get_state()) {
                    pls->state_lock.unlock();
                    continue;
                }
                pls->state_lock.unlock();
                send_party_room_destroy(reinterpret_cast<Player*>(pls), pl->get_indun_id());
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
    }
    else pl->vl.unlock();

    for (auto& other : my_vl) {
        Player* target = reinterpret_cast<Player*>(players[other]);
        if (true == is_npc(target->get_id())) continue;   // npc일 경우 보내지 않는다
        if (ST_INGAME != target->get_state() && ST_DEAD != target->get_state()) continue;
        target->vl.lock();
        if (0 != target->viewlist.count(c_id)) {
            target->viewlist.erase(c_id);
            target->vl.unlock();
            send_remove_object_packet(target, players[c_id]);
        }
        else target->vl.unlock();
    }

    // DB 연결
   
    if (players[c_id]->get_state() == ST_INGAME ||
        players[c_id]->get_state() == ST_DEAD) {
     //   EnterCriticalSection(&cs);
        Save_position(pl);
     //   LeaveCriticalSection(&cs);
    }
    

    // 이 파티원이 파티에 참가하고 있거나 레이드에 있으면 해제해주자


    players[c_id]->state_lock.lock();
    reinterpret_cast<Player*>(players[c_id])->CloseSocketPlayer();
    players[c_id]->set_state(ST_FREE);
    players[c_id]->state_lock.unlock();
}

void ObjectManager::worker()
{
    for (;;) {
        DWORD num_byte;
        LONG64 iocp_key;
        WSAOVERLAPPED* p_over;
        BOOL ret = GetQueuedCompletionStatus(*h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);
        int client_id = static_cast<int>(iocp_key);
        EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);
        if (FALSE == ret) {
            int err_no = WSAGetLastError();
            error_display(err_no);
            Player* pl = reinterpret_cast<Player*>(players[client_id]);
         //   Save_position(pl);// 차라리 disconnect 함수에 넣자 
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
            int remain_data = num_byte + pl->get_prev_size();
            unsigned char* packet_start = exp_over->_net_buf;
            int packet_size = packet_start[0];

            while (packet_size <= remain_data) {
                m_PacketManager->process_packet(pl, packet_start);
                remain_data -= packet_size;
                packet_start += packet_size;
                if (remain_data > 0) packet_size = packet_start[0];
                else break;
            }

            if (0 < remain_data) {
                pl->set_prev_size(remain_data);
                memcpy(&exp_over->_net_buf, packet_start, remain_data);
            }
            if (remain_data == 0) pl->set_prev_size(0);
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
            SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));
            int new_id = get_new_id();
            if (-1 == new_id) {
                cout << "Maxmum user overflow. Accept aborted.\n";
            }
            else {
                //players[new_id] = new Player(new_id);
                Player* pl = reinterpret_cast<Player*>(players[new_id]);
                pl->set_id(new_id);
                pl->accept_initialize();
                pl->set_tribe(HUMAN);
                //pl->_socket = c_socket;
                pl->set_socket(c_socket);

                CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), *h_iocp, new_id, 0);
                pl->do_recv();
            }

            ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
            c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            *(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
            AcceptEx(*s_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
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
            // target이 있는가?
            if (exp_over->_target == -1) {
                // 제자리로 돌아가는 것인가? 로밍인가?(npc_roming 함수에서 판단)
                players[client_id]->npc_roming(obstacles);
                //players[client_id]->return_npc_position(obstacles);
                m_SectorManager->player_move(players[client_id]); // 섹터 변경시 상태확인
                delete exp_over;
                break;
            }

            int target_id = exp_over->_target;
            players[target_id]->state_lock.lock();

            //쫒아가던 타겟이 살아있는가
            if (players[target_id]->get_state() != ST_INGAME) {
                players[target_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                players[client_id]->set_target_id(-1);
                players[client_id]->return_npc_position(obstacles);
                m_SectorManager->player_move(players[client_id]); // 섹터 변경시 상태확인
                delete exp_over;
                break;
            }
            players[target_id]->state_lock.unlock();

            // 타겟이 살아 있음(쫒아간다)
            if (players[client_id]->get_target_id() != -1) {
                players[client_id]->do_npc_move(players[exp_over->_target], obstacles);
                m_SectorManager->player_move(players[client_id]); // 섹터 변경시 상태확인
            }
            delete exp_over;

            break;
        }
        case OP_NPC_ATTACK: {
            // 죽은 상태나 공격하는 상태인지 아닌지 확인
            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME) || (false == players[client_id]->get_active())) {
                players[client_id]->state_lock.unlock();
                delete exp_over;
                break;
            }
            players[client_id]->state_lock.unlock();

            if (exp_over->_target < 0) {
                delete exp_over;
                break;
            }
            if (players[client_id]->npc_attack_validation(players[exp_over->_target])) {
                // 공격 성공

                // 죽었다면 섹터에서 제거해 주어야 함
                players[exp_over->_target]->state_lock.lock();
                if (players[exp_over->_target]->get_state() == ST_DEAD) {
                    players[exp_over->_target]->state_lock.unlock();
                    m_SectorManager->player_remove(players[exp_over->_target], true, players[client_id]);
                }
                else {  // target의 피 변화량을 주위 사람들에게 보내주어야함
                    players[exp_over->_target]->state_lock.unlock();
                }

            }
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
            pl->set_hp(pl->get_hp() + (pl->get_maxhp() * 0.1));
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
                TimerManager::timer_queue.push(ev);
            }
            send_change_hp_packet(pl, pl, 0);   // 플레이어 hp 자동 회복
            //send_status_change_packet(pl);
            break;
        }
        case OP_PLAYER_REVIVE: {
            if (reinterpret_cast<Player*>(players[client_id])->join_dungeon_room) {
                int indun_id = reinterpret_cast<Player*>(players[client_id])->get_indun_id();
                reinterpret_cast<Player*>(players[client_id])->revive_indun(dungeons[indun_id]);
            }
            else {
                reinterpret_cast<Player*>(players[client_id])->revive();
                // 섹터 처리
                m_SectorManager->player_put(players[client_id]);

            }
            delete exp_over;
            break;
        }
        case OP_NPC_REVIVE: {
            players[client_id]->revive();
            // 섹터 처리
            m_SectorManager->player_put(players[client_id]);
            players[client_id]->vl.lock();
            unordered_set<int>my_vl{ players[client_id]->viewlist };
            players[client_id]->vl.unlock();

            for (int i : my_vl) {
                if (static_ObjectManager::get_objManger()->get_player(i)->get_tribe() == HUMAN) {
                    players[client_id]->push_npc_move_event();
                    players[client_id]->set_move_active(true);
                    break;
                }
            }
            delete exp_over;
            break;
        }
        case OP_ELEMENT_COOLTIME: {
            timer_event ev;

            players[client_id]->state_lock.lock();
            if ((players[client_id]->get_state() != ST_INGAME)) {
                players[client_id]->state_lock.unlock();
                players[client_id]->set_active(false);
                delete exp_over;
                break;
            }
            players[client_id]->state_lock.unlock();

            if (exp_over->_target < 0) { delete exp_over; break; }

            switch (players[client_id]->get_element())
            {
            case E_WATER:
                players[exp_over->_target]->set_magical_attack(players[exp_over->_target]->get_magical_attack() / 9 * 10);
                break;
            case E_FULLMETAL:
                players[client_id]->set_physical_defence(players[client_id]->get_physical_defence() / 11 * 10);
                break;
            case E_WIND:
                reinterpret_cast<Player*>(players[client_id])->attack_speed_up = 0;
                break;
            case E_FIRE:
                if ((players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10) <= 0)
                    break;
                players[exp_over->_target]->set_hp(players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10);
                send_change_hp_packet(reinterpret_cast<Player*>(players[client_id]), players[exp_over->_target], players[exp_over->_target]->get_lv() * 10);
                if ((players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10) <= 0)
                    break;
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 5s;
                ev.ev = EVENT_ELEMENT_FIRE_COOLTIME;
                ev.target_id = exp_over->_target;
                TimerManager::timer_queue.push(ev);
                break;
            case E_TREE:
                players[exp_over->_target]->set_physical_attack(players[exp_over->_target]->get_physical_attack() / 9 * 10);
                break;
            case E_EARTH:
                players[client_id]->set_magical_defence(players[client_id]->get_magical_defence() / 11 * 10);
                break;
            case E_ICE:
                reinterpret_cast<Player*>(players[exp_over->_target])->attack_speed_up = 0;
                break;
            default:
                break;
            }
            if (players[client_id]->get_element() != E_FIRE) 
                players[exp_over->_target]->set_element_cooltime(false);
                delete exp_over;
            
            break;
        }
        case OP_BOSS_MOVE: {
            dungeons[client_id]->state_lock.lock();
            if (dungeons[client_id]->get_dun_st() != DUN_ST_START) {
                dungeons[client_id]->state_lock.unlock();
                break;
            }
            dungeons[client_id]->state_lock.unlock();

            dungeons[client_id]->boss_move();
            timer_event ev;
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 500ms;
            ev.ev = EVENT_BOSS_MOVE;
            ev.target_id = -1;
            TimerManager::timer_queue.push(ev);
            delete exp_over;
            break;
        }
        case OP_BOSS_ATTACK: {
            dungeons[client_id]->state_lock.lock();
            if (dungeons[client_id]->get_dun_st() != DUN_ST_START) {
                dungeons[client_id]->state_lock.unlock();
                break;
            }
            dungeons[client_id]->state_lock.unlock();
            dungeons[client_id]->boss_attack();
            delete exp_over;
            break;
        }
        case OP_GAIA_PATTERN: {
            dungeons[client_id]->state_lock.lock();
            if (dungeons[client_id]->get_dun_st() != DUN_ST_START) {
                dungeons[client_id]->state_lock.unlock();
                break;
            }
            dungeons[client_id]->state_lock.unlock();
            dungeons[client_id]->pattern_active(exp_over->_target);
            delete exp_over;
            break;
        }
        case OP_PARTNER_MOVE: {
            players[client_id]->state_lock.lock();
            if (players[client_id]->get_state() != ST_INDUN) {
                players[client_id]->state_lock.unlock();
                break;
            }
            players[client_id]->state_lock.unlock();

            Partner* pl = reinterpret_cast<Partner*>(players[client_id]);
            pl->partner_move(pl, dungeons[pl->get_indun_id()]);
            Player** pp = dungeons[pl->get_indun_id()]->get_party_palyer();
            for (int i = 0; i < GAIA_ROOM; i++) {
                send_move_packet(pp[i], pl, 1);
                send_look_packet(pp[i], pl);
            }

            timer_event ev;
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 500ms;
            ev.ev = EVENT_PARTNER_MOVE;
            ev.target_id = -1;
            TimerManager::timer_queue.push(ev);
            delete exp_over;
            break;
        }
        case OP_PARTNER_SKILL: {
            players[client_id]->state_lock.lock();
            if (players[client_id]->get_state() != ST_INDUN) {
                players[client_id]->state_lock.unlock();
                break;
            }
            players[client_id]->state_lock.unlock();

            int indun_id = reinterpret_cast<Player*>(players[client_id])->get_indun_id();  //바꿔야함!


            switch (reinterpret_cast<Player*>(players[client_id])->get_job())
            {
            case J_DILLER: {
                for (int i = 0; i < GAIA_ROOM; ++i) {
                    dungeons[indun_id]->get_party_palyer()[i]->set_physical_attack(dungeons[indun_id]->get_party_palyer()[i]->get_origin_physical_attack());
                    dungeons[indun_id]->get_party_palyer()[i]->set_magical_attack(dungeons[indun_id]->get_party_palyer()[i]->get_origin_magical_attack());
                }
                break;
            }
            case J_TANKER: {
                for (int i = 0; i < GAIA_ROOM; ++i) {
                    dungeons[indun_id]->get_party_palyer()[i]->set_physical_defence(dungeons[indun_id]->get_party_palyer()[i]->get_origin_physical_defence());
                    dungeons[indun_id]->get_party_palyer()[i]->set_magical_defence(dungeons[indun_id]->get_party_palyer()[i]->get_origin_magical_defence());
                }
                break;
            }
            case J_SUPPORTER: {
                for (int i = 0; i < GAIA_ROOM; ++i) {
                    dungeons[indun_id]->get_party_palyer()[i]->attack_speed_up = 0;
                }
                break;
            }
            case J_MAGICIAN: {
                break;
            }
            }
            Partner* pl = reinterpret_cast<Partner*>(players[client_id]);
            pl->partner_attack(pl, dungeons[pl->get_indun_id()]);
            delete exp_over;
            break;
        }
        case OP_PARTNER_SKILL_STOP: {
            players[client_id]->state_lock.lock();
            if (players[client_id]->get_state() != ST_INDUN) {
                players[client_id]->state_lock.unlock();
                break;
            }
            players[client_id]->state_lock.unlock();

            int indun_id = reinterpret_cast<Player*>(players[client_id])->get_indun_id();  

            for (int i = 0; i < GAIA_ROOM; ++i) {
                if (dungeons[indun_id]->get_party_palyer()[i]->get_id() == exp_over->_target)
                    reinterpret_cast<Partner*>(dungeons[indun_id]->get_party_palyer()[i])->running_pattern = false;
            }
            break;
        }
        case OP_PARTNER_ATTACK_STOP: {
            players[client_id]->state_lock.lock();
            if (players[client_id]->get_state() != ST_INDUN) {
                players[client_id]->state_lock.unlock();
                break;
            }
            players[client_id]->state_lock.unlock();

            int indun_id = reinterpret_cast<Player*>(players[client_id])->get_indun_id();

            for (int i = 0; i < GAIA_ROOM; ++i) {
                if (dungeons[indun_id]->get_party_palyer()[i]->get_id() == exp_over->_target)
                    reinterpret_cast<Partner*>(dungeons[indun_id]->get_party_palyer()[i])->running_attack = false;
            }
            break;
        }
        case OP_PARTNER_NORMAL_ATTACK: {
            players[client_id]->state_lock.lock();
            if (players[client_id]->get_state() != ST_INDUN) {
                players[client_id]->state_lock.unlock();
                break;
            }
            players[client_id]->state_lock.unlock();
            Partner* pl = reinterpret_cast<Partner*>(players[client_id]);
            pl->partner_normal_attack(pl, dungeons[pl->get_indun_id()]);

            timer_event ev;
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_PARTNER_NORMAL_ATTACK;
            ev.target_id = 1;
            TimerManager::timer_queue.push(ev);

            pl->running_attack = true;
            switch (pl->get_job())
            {
            case J_DILLER:
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() +3ms;
                ev.ev = EVENT_PARTNER_ATTACK_STOP;
                ev.target_id = client_id;
                TimerManager::timer_queue.push(ev);
                break;
            case J_TANKER:
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() +  2s + 2ms;
                ev.ev = EVENT_PARTNER_ATTACK_STOP;
                ev.target_id = client_id;
                TimerManager::timer_queue.push(ev);
                break;
            case J_MAGICIAN:
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() + 1s;
                ev.ev = EVENT_PARTNER_ATTACK_STOP;
                ev.target_id = client_id;
                TimerManager::timer_queue.push(ev);
                break;
            case J_SUPPORTER:
                ev.obj_id = client_id;
                ev.start_time = chrono::system_clock::now() +  2s;
                ev.ev = EVENT_PARTNER_ATTACK_STOP;
                ev.target_id = client_id;
                TimerManager::timer_queue.push(ev);
                break;
            }

            delete exp_over;
            break;
        }
        case OP_GAMESTART_TIMER: {
            Gaia* dun = dungeons[exp_over->_target];
            dun->game_start();
            dun->state_lock.lock();
            if (dun->get_dun_st() == DUN_ST_START) {
                dun->state_lock.unlock();
                // 게임이 시작 되었으니 시야처리를 해주자
                Player** vl_pl;
                vl_pl = dun->get_party_palyer();

                unordered_set<int> indun_vl;
                for (int j = 0; j < GAIA_ROOM; j++) indun_vl.insert(vl_pl[j]->get_id());

                for (int j = 0; j < GAIA_ROOM; j++) {
                    if (vl_pl[j]->get_tribe() != HUMAN) continue;
                    vl_pl[j]->vl.lock();
                    unordered_set<int>temp_vl{ vl_pl[j]->viewlist };
                    vl_pl[j]->viewlist = indun_vl;
                    vl_pl[j]->viewlist.erase(vl_pl[j]->get_id());
                    vl_pl[j]->vl.unlock();

                    for (auto k : temp_vl) {
                        send_remove_object_packet(vl_pl[j], players[k]);
                        if (is_npc(k) == true) continue;
                        reinterpret_cast<Player*>(players[k])->vl.lock();
                        if (indun_vl.find(k) == indun_vl.end()) {
                            reinterpret_cast<Player*>(players[k])->viewlist.erase(client_id);
                            reinterpret_cast<Player*>(players[k])->vl.unlock();
                            send_remove_object_packet(reinterpret_cast<Player*>(players[k]), vl_pl[j]);
                            continue;
                        }
                        reinterpret_cast<Player*>(players[k])->vl.unlock();
                    }

                    for (auto k : indun_vl) {
                        if (k == vl_pl[j]->get_id()) continue;
                        send_put_object_packet(vl_pl[j], players[k]);
                    }
                    send_put_object_packet(vl_pl[j], dun->boss);
                }
                break;
            }
            dun->state_lock.unlock();
            delete exp_over;
            break;
        }
        case OP_FINISH_RAID: {
            Gaia* dun = dungeons[client_id];
            Player** party_players = dun->get_party_palyer();

            // 플레이어는 오픈 월드로 되돌리기
            int human_player = 0;
            for (int i = 0; i < (GAIA_ROOM - human_player); i++) {
                if (party_players[i]->get_tribe() == HUMAN) {
                    party_players[i]->join_dungeon_room == false;
                    // 원래는 DB에서 받아와야 하는 정보를 기본 정보로 대체
                    party_players[i]->set_x(3210);
                    party_players[i]->set_y(0);
                    party_players[i]->set_z(940);
                    party_players[i]->set_hp(party_players[i]->get_maxhp());
                    party_players[i]->indun_id - 1;

                    // 오픈월드로 이동한다는 패킷 보내주기
                    send_move_openwrold(party_players[i]);

                    // 오픈월드로 위치 이동 및 시야처리
                    party_players[i]->state_lock.lock();
                    if (party_players[i]->get_state() == ST_DEAD) {
                        send_revive_packet(party_players[i], party_players[i]);
                    }
                    party_players[i]->set_state(ST_INGAME);
                    party_players[i]->state_lock.unlock();

                    m_SectorManager->player_put(party_players[i]);
                    send_status_change_packet(party_players[i]);

                    dun->quit_palyer(party_players[i]);
                    i--;
                    human_player++;
                }
            }


            // 방 파괴되기전 처리
            dun->state_lock.lock();
            dun->set_dun_st(DUN_ST_FREE);
            dun->state_lock.unlock();
            for (auto& pls : players) {
                if (true == is_npc(pls->get_id())) break;
                pls->state_lock.lock();
                if (ST_INGAME != pls->get_state()) {
                    pls->state_lock.unlock();
                    continue;
                }
                pls->state_lock.unlock();
                send_party_room_destroy(reinterpret_cast<Player*>(pls), client_id);
            }

            // AI는 해제해주기
            for (int i = 0; i < dun->player_cnt; i++) {
                int delete_id = party_players[i]->get_id();
                players[delete_id]->state_lock.lock();
                players[delete_id]->set_state(ST_FREE);
                players[delete_id]->state_lock.unlock();
            }

            // 방 파괴해주기
            dun->destroy_dungeon();

            break;
        }
        case OP_PLAYER_ATTACK: {
            reinterpret_cast<Player*>(players[client_id])->set_attack_active(false);
            break;
        }
        case OP_SKILL_COOLTIME: {
            if (exp_over->_target == 2) {  // 전사 BUFF
                switch (reinterpret_cast<Player*>(players[client_id])->get_job())
                {
                case J_DILLER: {
                    players[client_id]->set_physical_attack(0.3 * players[client_id]->get_lv() * players[client_id]->get_lv() + 10 * players[client_id]->get_lv());
                    players[client_id]->set_magical_attack(0.1 * players[client_id]->get_lv() * players[client_id]->get_lv() + 5 * players[client_id]->get_lv());
                    break;
                }
                case J_TANKER: {
                    players[client_id]->set_physical_defence(0.27 * players[client_id]->get_lv() * players[client_id]->get_lv() + 10 * players[client_id]->get_lv());
                    players[client_id]->set_magical_defence(0.2 * players[client_id]->get_lv() * players[client_id]->get_lv() + 10 * players[client_id]->get_lv());
                    break;
                }
                case J_MAGICIAN: {  //없음 

                    break;
                }
                case J_SUPPORTER: {   // 대상이 여러명일 때는 어떻게 다시 초기화할까 
                    if (dungeons[client_id]->start_game == false) {
                        for (int i = 0; i < MAX_USER; ++i) {
                            reinterpret_cast<Player*>(players[i])->attack_speed_up = 0;
                        }
                    }
                    else {
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            dungeons[client_id]->get_party_palyer()[i]->attack_speed_up = 0;
                        }
                    }
                    break;
                }
                }
                // 일단 이것을 넣으면 안돌아감(이유 모름)
                //send_status_change_packet(reinterpret_cast<Player*>(players[ev.obj_id]));
            }
            reinterpret_cast<Player*>(players[client_id])->set_skill_active(exp_over->_target, false);
            break;
        }
        case OP_ELEMENT_FIRE_COOLTIME: {
            if ((players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10) <= 0)
                break;
            players[exp_over->_target]->set_hp(players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10);
            send_change_hp_packet(reinterpret_cast<Player*>(players[client_id]), players[exp_over->_target], players[exp_over->_target]->get_lv() * 10);
            // players[client_id]->set_element_cooltime(false);
            if ((players[exp_over->_target]->get_hp() - players[exp_over->_target]->get_lv() * 10) <= 0)
                break;
            players[exp_over->_target]->set_element_cooltime(false);
            delete exp_over;
            break;
        }
        }

    }
}

void ObjectManager::set_packetManager(PacketManager* packetManager)
{
    m_PacketManager = packetManager;
    m_PacketManager->set_players_object(players);
}

int ObjectManager::get_new_id()
{
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

int ObjectManager::get_new_ai_id()
{
    for (int i = AI_ID_START; i < AI_ID_END; ++i) {
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

bool ObjectManager::is_near(int a, int b)
{
    if (RANGE < abs(players[a]->get_x() - players[b]->get_x())) return false;
    if (RANGE < abs(players[a]->get_z() - players[b]->get_z())) return false;
    return true;
}

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

bool ObjectManager::check_move_alright(int x, int z, bool monster)
{
    if (x <= 0 || WORLD_WIDTH <= x) return false;
    if (z <= 67 || WORLD_HEIGHT <= z) return false;

    // 성벽 충돌처리
    if (z <= 1000) {
        if (x <= 2369) return false;
    }

    if (z <= 1000 && z >= 948) {
        if (x <= 3164 || x >= 3223) return false;
    }

    // 집 충돌처리
    for (int i = 0; i < 12; i++) {
        if (isInsideTriangle(Coord(house_obstacles[4 * i]->get_x(), house_obstacles[4 * i]->get_z()),
            Coord(house_obstacles[4 * i + 1]->get_x(), house_obstacles[4 * i + 1]->get_z()),
            Coord(house_obstacles[4 * i + 2]->get_x(), house_obstacles[4 * i + 2]->get_z()),
            Coord(x, z))) return false;
        if (isInsideTriangle(Coord(house_obstacles[4 * i + 2]->get_x(), house_obstacles[4 * i + 2]->get_z()),
            Coord(house_obstacles[4 * i + 3]->get_x(), house_obstacles[4 * i + 3]->get_z()),
            Coord(house_obstacles[4 * i]->get_x(), house_obstacles[4 * i]->get_z()),
            Coord(x, z)))  return false;
    }

    // 우물
    if (sqrt(pow(x - 3333, 2) + pow(z - 596, 2)) < 30) return false;

    int size = 0;
    if (monster) size = 15;
    else size = 5;

    for (auto ob : obstacles) {
        if ((ob->get_x() - size <= x && x <= ob->get_x() + size) && 
            (ob->get_z() - size <= z && z <= ob->get_z() + size)) {
            return false;
        }
    }
    return true;
}

bool ObjectManager::check_move_alright_indun(int x, int z)
{
    int m_x = 2037;
    int m_z = 2112;
    float r = 515.f;
    
    // 중심과 (x,z)의 거리가 r보다 작으면 true
    if (sqrt(pow((x - m_x), 2) + pow((z - m_z), 2)) < r) return true;
    else return false;

}

Npc* ObjectManager::get_player(int c_id)
{
    return players[c_id];
}

Gaia* ObjectManager::get_dungeon(int d_id)
{
    return dungeons[d_id];
}

array<Gaia*, MAX_DUNGEONS>& ObjectManager::get_dungeons()
{
    return dungeons;
}

void ObjectManager::CloseServer()
{
    for (auto& pl : players) {
        if (pl->get_tribe() != HUMAN) break;
        if (ST_INGAME == pl->get_state())
            if (ST_INGAME == pl->get_state()) {
                Save_position(reinterpret_cast<Player*>(pl));
                Disconnect(pl->get_id());
            }
    }
}

// static_objectmanagr
ObjectManager* static_ObjectManager::get_objManger()
{
    return objManager;
}

void* static_ObjectManager::set_objManger(ObjectManager* om)
{
    objManager = om;
    return 0;
}

Npc* static_ObjectManager::get_player(int c_id)
{
    return objManager->get_player(c_id);
}
