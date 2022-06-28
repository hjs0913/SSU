#include "PacketManager.h"
#include "send.h"

PacketManager::PacketManager(ObjectManager* objectManager, HANDLE* iocp)
{
	m_ObjectManger = objectManager;
    h_iocp = iocp;
}

void PacketManager::process_packet(int client_id, unsigned char* p)
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

        for (auto* p : players) {
            if (p->get_tribe() != HUMAN) break;
            if (p->get_state() == ST_FREE) continue;
            if (p->get_id() == client_id) continue;
            if (strcmp(packet->id, "admin") == 0) break;

            if (strcmp(reinterpret_cast<Player*>(p)->get_login_id(), packet->id) == 0) {
                cout << "중복된 아이디 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
                m_ObjectManger->Disconnect(client_id);
                return;
            }
            if (strcmp(reinterpret_cast<Player*>(p)->get_name(), packet->name) == 0) {
                cout << "중복된 닉네임 접속 확인" << endl;
                send_login_fail_packet(pl, 1);   // 중복 로그인
                m_ObjectManger->Disconnect(client_id);
                return;
            }

        }
        // 원래는 DB에서 받아와야 하는 정보를 기본 정보로 대체
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


        // Stress Test용
        if (strcmp(packet->id, "admin") == 0) {
            pl->set_x(rand() % 4000);
            pl->set_z(rand() % 4000);
        }

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
        case J_SUPPORTER: {
            int lv = pl->get_lv();
            pl->set_maxhp(18 * lv * lv + 70 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(15 * lv * lv + 60 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_magical_attack(0.25 * lv * lv + 8 * lv);
            pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
            break;
        }
        case J_MAGICIAN: {
            int lv = pl->get_lv();
            pl->set_maxhp(16 * lv * lv + 70 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(17 * lv * lv + 60 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_magical_attack(0.3 * lv * lv + 10 * lv);
            pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
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

            if (true == is_npc(other->get_id())) break;

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
        pl->viewlist.clear();
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

        pl->state_lock.lock();
        if (pl->get_state() == ST_DEAD || pl->get_state() == ST_FREE) {
            pl->state_lock.unlock();
            break;
        }
        pl->state_lock.unlock();
        // InDunProcess
        if (pl->get_state() == ST_INDUN) {
            // 유효성 검사
            //if (check_move_alright(x, z, false) == false) { // Raid Map에 맞는 유효성 검사 필요
            //    send_move_packet(pl, pl);   
            //    break;
            //}
            pl->set_x(x);
            pl->set_y(y);
            pl->set_z(z);
            pl->vl.lock();
            unordered_set <int> my_vl{ pl->viewlist };
            pl->vl.unlock();
            send_move_packet(pl, pl, 1);
            for (auto& other : players) {
                if (other->get_state() != ST_INDUN) continue;
                if (is_npc(other->get_id())) break;
                if (reinterpret_cast<Player*>(other)->indun_id == pl->indun_id) {
                    if (other->get_id() == pl->get_id()) continue;
                    send_move_packet(reinterpret_cast<Player*>(other), pl, 1);
                }
            }
            break;
        }


        // 유효성 검사
        if (check_move_alright(x, z, false) == false) {
            // 올바르지 않을경우 위치를 수정을 해주어야 한다
            send_move_packet(pl, pl, 0);
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

        send_move_packet(pl, pl, 1); // 내 자신의 움직임을 먼저 보내주자

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
                    send_move_packet(other_player, pl, 1);
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
                    send_move_packet(other_player, pl, 1);
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
            if (players[vl_id]->get_tribe() == HUMAN) {
                send_animation_attack(reinterpret_cast<Player*>(players[vl_id]), pl->get_id());
            }
        }

        timer_event ev;
        if (pl->attack_speed_up == false) {
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_PLAYER_ATTACK;
            ev.target_id = client_id;
            timer_queue.push(ev);
        }
        else {
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 50ms;
            ev.ev = EVENT_PLAYER_ATTACK;
            ev.target_id = client_id;
            timer_queue.push(ev);
        }

        if (pl->join_dungeon_room) {
            int indun = pl->indun_id;
            Npc* bos = dungeons[indun]->boss;
            if (bos->get_x() >= pl->get_x() - 20 && bos->get_x() <= pl->get_x() + 20) {
                if (bos->get_z() >= pl->get_z() - 20 && bos->get_z() <= pl->get_z() + 20) {
                    // 일단 고정값으로 제거해 주자
                    //bos->set_hp(bos->get_hp() - 130000);
                    if (bos->get_hp() > 0) {
                        attack_success(pl, bos, pl->get_basic_attack_factor());

                        Player** ps = dungeons[indun]->get_party_palyer();
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            send_change_hp_packet(ps[i], bos);
                        }

                        if (bos->get_hp() < 0) {
                            bos->set_hp(0);
                            dungeons[indun]->game_victory();
                        }
                    }

                }
            }
            return;
        }

        for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
            players[i]->state_lock.lock();
            if (players[i]->get_state() != ST_INGAME) {
                players[i]->state_lock.unlock();
                continue;
            }
            players[i]->state_lock.unlock();
            if (players[i]->get_x() >= pl->get_x() - 20 && players[i]->get_x() <= pl->get_x() + 20) {
                if (players[i]->get_z() >= pl->get_z() - 20 && players[i]->get_z() <= pl->get_z() + 20) {
                    attack_success(pl, players[i], pl->get_basic_attack_factor());    // 데미지 계산
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
        if (pl->join_dungeon_room) {
            if (dungeons[pl->get_indun_id()]->get_dun_st() == DUN_ST_START) {
                Player** vl_pl;
                vl_pl = dungeons[pl->indun_id]->get_party_palyer();
                for (int i = 0; i < GAIA_ROOM; i++) {
                    if (vl_pl[i]->get_state() == ST_INGAME || vl_pl[i]->get_state() == ST_INDUN) {
                        send_chat_packet(vl_pl[i], client_id, c_temp);
                    }
                    else break;
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
        cout << client_id << endl;
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
    case CS_PACKET_SKILL: {

        if (pl->get_mp() - 1000 < 0)  //mp없으면 안됨 
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
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 0;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
                    send_status_change_packet(pl);
                    if (!pl->join_dungeon_room) {
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
                                //send_status_change_packet(pl);
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
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Npc* bos = dungeons[indun_id]->boss;
                        if ((bos->get_x() >= pl->get_x() - 10 && bos->get_x() <= pl->get_x() + 10) && (bos->get_z() >= pl->get_z() - 10 && bos->get_z() <= pl->get_z() + 10)) {
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
                                    send_change_hp_packet(dungeons[client_id]->get_party_palyer()[i], dungeons[client_id]->boss);
                                }
                                if (bos->get_hp() < 0) {
                                    bos->set_hp(0);
                                    dungeons[indun_id]->game_victory();
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
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 1;
                    timer_queue.push(ev);


                    Coord a = { pl->get_x(), pl->get_z() };    //플레이어 기준 전방 삼각형 범위 
                    Coord b = { pl->get_x() - pl->get_right_x() * 40 + pl->get_look_x() * 100,
                        pl->get_z() - pl->get_right_z() * 40 + pl->get_look_z() * 100 };  // 왼쪽 위
                    Coord c = { pl->get_x() + pl->get_right_x() * 40 + pl->get_look_x() * 100,
                        pl->get_z() + pl->get_right_z() * 40 + pl->get_look_z() * 100 };  // 오른쪽 위

                    pl->set_mp(pl->get_mp() - 1000);
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
                                pl->set_skill_factor(packet->skill_type, packet->skill_num);
                                magical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));
                                players[i]->set_target_id(pl->get_id());
                                //send_status_change_packet(pl);
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
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Npc* bos = dungeons[indun_id]->boss;
                        Coord n = { bos->get_x(), bos->get_z() };

                        if (isInsideTriangle(a, b, c, n)) {
                            pl->set_skill_factor(1, 0);
                            float give_damage = pl->get_magical_attack() * pl->get_skill_factor(1, 0);
                            float defence_damage = (bos->get_defence_factor() *
                                bos->get_magical_defence()) / (1 + (bos->get_defence_factor() *
                                    bos->get_magical_defence()));
                            float damage = give_damage * (1 - defence_damage);
                            if (bos->get_hp() > 0) {
                                bos->set_hp(bos->get_hp() - damage);
                                //send_status_change_packet(pl);
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    send_change_hp_packet(dungeons[client_id]->get_party_palyer()[i], dungeons[client_id]->boss);
                                }
                                if (bos->get_hp() < 0) {
                                    bos->set_hp(0);
                                    dungeons[indun_id]->game_victory();
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
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 0;
                    timer_queue.push(ev);


                    pl->set_mp(pl->get_mp() - 1000);
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
                                physical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor(packet->skill_type, packet->skill_num));

                                players[i]->set_pos(players[i]->get_x() + pl->get_look_x() * 40, players[i]->get_z() + pl->get_look_z() * 40);
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
                                    timer_queue.push(ev);

                                    Activate_Npc_Move_Event(i, pl->get_id());
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        Npc* bos = dungeons[indun_id]->boss;
                        if ((bos->get_x() >= pl->get_x() - 15 && bos->get_x() <= pl->get_x() + 15) &&
                            (bos->get_z() >= pl->get_z() - 15 && bos->get_z() <= pl->get_z() + 15)) {
                            pl->set_skill_factor(0, 0);
                            float give_damage = pl->get_physical_attack() * pl->get_skill_factor(0, 0);
                            float defence_damage = (bos->get_defence_factor() *
                                bos->get_physical_defence()) / (1 + (bos->get_defence_factor() *
                                    bos->get_physical_defence()));
                            float damage = give_damage * (1 - defence_damage);
                            if (bos->get_hp() > 0) {
                                bos->set_pos(dungeons[client_id]->boss->get_x() + pl->get_look_x() * 100, dungeons[client_id]->boss->get_z() + pl->get_look_z() * 100);
                                bos->set_hp(dungeons[client_id]->boss->get_hp() - damage);
                                //send_move_packet(pl, dungeons[client_id]->boss, 1);  //나중에 수정필요 
                                for (int i = 0; i < GAIA_ROOM; ++i) {
                                    send_change_hp_packet(dungeons[client_id]->get_party_palyer()[i], dungeons[client_id]->boss);
                                }
                                send_status_change_packet(pl);
                                if (bos->get_hp() < 0) {
                                    bos->set_hp(0);
                                    dungeons[indun_id]->game_victory();
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
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 3s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 1;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
                                    timer_queue.push(ev);
                                    Activate_Npc_Move_Event(i, pl->get_id());
                                }
                            }
                        }
                    }
                    else {
                        int indun_id = pl->get_indun_id();
                        if ((dungeons[indun_id]->get_x() >= pl->get_x() - 40 && dungeons[indun_id]->get_x() <= pl->get_x() + 40) &&
                            (dungeons[indun_id]->get_z() >= pl->get_z() - 40 && dungeons[indun_id]->get_z() <= pl->get_z() + 40)) {
                            pl->set_skill_factor((int)packet->skill_type, (int)packet->skill_num);
                            dungeons[indun_id]->target_id = pl->get_indun_id();
                        }
                    }
                    break;
                }
                break;
            case 2:  //버프  방어력 증가 
                switch ((int)packet->skill_num)
                {
                case 0:
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 10s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
            case 2: //버프
                switch (packet->skill_num)
                {
                case 0: {// 사각형 내부 범위 플레이어 hp 회복  
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
                        int tmp_hp = 0;
                        int target_player = 0;
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            if (i == 0) {
                                target_player = i;
                                tmp_hp = dungeons[indun_id]->get_party_palyer()[i]->get_hp();
                            }
                            else {
                                if (tmp_hp > dungeons[indun_id]->get_party_palyer()[i]->get_hp()) {
                                    target_player = i;
                                    tmp_hp = dungeons[indun_id]->get_party_palyer()[i]->get_hp();
                                }
                            }
                        }
                        pl->set_mp(pl->get_mp() - 1000);
                        send_status_change_packet(pl);
                        send_buff_ui_packet(dungeons[indun_id]->get_party_palyer()[target_player], 2); //ui
                        dungeons[indun_id]->get_party_palyer()[target_player]->set_hp(dungeons[indun_id]->get_party_palyer()[target_player]->get_hp() + dungeons[indun_id]->get_party_palyer()[target_player]->get_maxhp() / 10);
                        if (dungeons[indun_id]->get_party_palyer()[target_player]->get_hp() > dungeons[indun_id]->get_party_palyer()[target_player]->get_maxhp())
                            dungeons[indun_id]->get_party_palyer()[target_player]->set_hp(dungeons[indun_id]->get_party_palyer()[target_player]->get_maxhp());

                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            send_change_hp_packet(dungeons[indun_id]->get_party_palyer()[i], dungeons[indun_id]->get_party_palyer()[target_player]);
                        }
                    }
                    break;
                }
                case 1: {  //mp회복 
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
                        int tmp_mp = 0;
                        int target_player = 0;
                        for (int i = 0; i < GAIA_ROOM; i++) {
                            if (i == 0) {
                                target_player = i;
                                tmp_mp = dungeons[indun_id]->get_party_palyer()[i]->get_mp();
                            }
                            else {
                                if (tmp_mp > dungeons[indun_id]->get_party_palyer()[i]->get_mp()) {
                                    target_player = i;
                                    tmp_mp = dungeons[indun_id]->get_party_palyer()[i]->get_mp();
                                }
                            }
                        }
                        pl->set_mp(pl->get_mp() - 1000);
                        send_status_change_packet(pl);
                        send_buff_ui_packet(dungeons[indun_id]->get_party_palyer()[target_player], 0); //ui
                        dungeons[indun_id]->get_party_palyer()[target_player]->set_mp(dungeons[indun_id]->get_party_palyer()[target_player]->get_mp() + dungeons[indun_id]->get_party_palyer()[target_player]->get_maxmp() / 10);
                        if (dungeons[indun_id]->get_party_palyer()[target_player]->get_mp() > dungeons[indun_id]->get_party_palyer()[target_player]->get_maxmp())
                            dungeons[indun_id]->get_party_palyer()[target_player]->set_mp(dungeons[indun_id]->get_party_palyer()[target_player]->get_maxmp());
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            send_change_mp_packet(dungeons[indun_id]->get_party_palyer()[i], dungeons[indun_id]->get_party_palyer()[target_player]);
                        }
                    }
                    break;
                }

                case 2: {//공속 
                    timer_event ev;
                    ev.obj_id = client_id;
                    ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
                    ev.ev = EVENT_SKILL_COOLTIME;
                    ev.target_id = 2;
                    timer_queue.push(ev);

                    pl->set_mp(pl->get_mp() - 1000);
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
                                reinterpret_cast<Player*>(players[i])->attack_speed_up = true;
                                send_buff_ui_packet(reinterpret_cast<Player*>(players[i]), 4);
                            }
                        }
                    }
                    else {
                        for (int i = 0; i < GAIA_ROOM; ++i) {
                            dungeons[pl->get_indun_id()]->get_party_palyer()[i]->attack_speed_up = true;
                            send_buff_ui_packet(dungeons[pl->get_indun_id()]->get_party_palyer()[i], 4);
                        }
                    }
                    break;
                }
                      break;
                }
                break;
            }
            for (int vl_id : my_vl) {
                send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num);
            }
            send_animation_skill(pl, pl->get_id(), (int)packet->skill_num);
            break;
        }
        case J_MAGICIAN: {
            timer_event ev;
            ev.obj_id = client_id;
            ev.start_time = chrono::system_clock::now() + 5s;  //쿨타임
            ev.ev = EVENT_SKILL_COOLTIME;
            ev.target_id = 1;
            timer_queue.push(ev);
            switch ((int)packet->skill_type)
            {
            case 0:
                break;
            case 1: //마법
                switch ((int)packet->skill_num)
                {

                case 0: // mp흡수 

                    pl->set_hp(pl->get_hp() - 300);
                    send_status_change_packet(pl);

                    for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                        players[i]->state_lock.lock();
                        if (players[i]->get_state() != ST_INGAME) {
                            players[i]->state_lock.unlock();
                            continue;
                        }
                        players[i]->state_lock.unlock();



                        if ((players[i]->get_x() >= pl->get_x() - 10 && players[i]->get_x() <= pl->get_x() + 10) && (players[i]->get_z() >= pl->get_z() - 10 && players[i]->get_z() <= pl->get_z() + 10)) {

                            pl->set_mp(pl->get_mp() + players[i]->get_hp() / 10);
                            if (pl->get_mp() > pl->get_maxmp())
                                pl->set_mp(pl->get_maxmp());


                            pl->set_skill_factor((int)packet->skill_type, (int)packet->skill_num);
                            magical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor((int)packet->skill_type, (int)packet->skill_num));
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
                                timer_queue.push(ev);

                                Activate_Npc_Move_Event(i, pl->get_id());
                            }
                        }
                    }
                    break;
                case 1:

                    send_play_shoot_packet(pl); // 쏘라고 보내줘야 쏘자 

                    pl->set_mp(pl->get_mp() - 1500);
                    send_status_change_packet(pl);

                    Coord a = { pl->get_x() + pl->get_right_x() * -10, pl->get_z() + pl->get_right_z() * -10 };
                    Coord b = { pl->get_x() + pl->get_right_x() * 10, pl->get_z() + pl->get_right_z() * 10 };
                    Coord c = { (pl->get_x() + pl->get_right_x() * -10) + pl->get_look_x() * 100,
                   (pl->get_z() + pl->get_right_z() * -10) + pl->get_look_z() * 100, };


                    Coord d = { pl->get_x() + pl->get_right_x() * 10, pl->get_z() + pl->get_right_z() * 10 };
                    Coord e = { (pl->get_x() + pl->get_right_x() * 10) + pl->get_look_x() * 100
                        , (pl->get_z() + pl->get_right_z() * 10) + pl->get_look_x() * 100 };
                    Coord f = { (pl->get_x() + pl->get_right_x() * -10) + pl->get_look_x() * 100,
                   (pl->get_z() + pl->get_right_z() * -10) + pl->get_look_z() * 100, };




                    for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
                        players[i]->state_lock.lock();
                        if (players[i]->get_state() != ST_INGAME) {
                            players[i]->state_lock.unlock();
                            continue;
                        }
                        players[i]->state_lock.unlock();

                        Coord n = { players[i]->get_x(), players[i]->get_z() };
                        //  players[i]->set_pos(players[i]->get_x() + pl->get_look_x() * 40 , players[i]->get_z() + pl->get_look_z() * 40);


                         //if (players[i]->get_x() < pl->get_x() + pl->get_look_x() * 20 &&  players[i]->get_z() < pl->get_z()  + pl->get_look_z() * 100) {
                        if (isInsideTriangle(a, b, c, n) || isInsideTriangle(d, e, f, n)) {

                            pl->set_skill_factor((int)packet->skill_type, (int)packet->skill_num);
                            players[i]->set_target_id(pl->get_id());
                            magical_skill_success(client_id, players[i]->get_id(), pl->get_skill_factor((int)packet->skill_type, (int)packet->skill_num));
                            send_play_effect_packet(pl, players[i]); // 이펙트 터트릴 위치 



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
            }
            break;
        }
                       break;
        }
        for (int vl_id : my_vl) {
            send_animation_skill(reinterpret_cast<Player*>(players[vl_id]), pl->get_id(), (int)packet->skill_num);
        }
        send_animation_skill(pl, pl->get_id(), (int)packet->skill_num);
    }
                        break;
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
            if (is_npc(i) == true) continue;
            // Player
            send_look_packet(reinterpret_cast<Player*>(players[i]), pl);
        }
        break;
    }
    case CS_PACKET_CHANGE_JOB: {
        cs_packet_change_job* packet = reinterpret_cast<cs_packet_change_job*>(p);
        pl->set_job(packet->job);

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
        case J_SUPPORTER: {
            int lv = pl->get_lv();
            pl->set_maxhp(18 * lv * lv + 70 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(15 * lv * lv + 60 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_magical_attack(0.25 * lv * lv + 8 * lv);
            pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
            break;
        }
        case J_MAGICIAN: {
            int lv = pl->get_lv();
            pl->set_maxhp(16 * lv * lv + 70 * lv);
            pl->set_hp(pl->get_maxhp());
            pl->set_maxmp(17 * lv * lv + 60 * lv);
            pl->set_mp(pl->get_maxmp());
            pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
            pl->set_magical_attack(0.3 * lv * lv + 10 * lv);
            pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
            pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
            pl->set_basic_attack_factor(50.0f);
            pl->set_defence_factor(0.0002);
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
                ev.target_id = 0; // packet->target;
                timer_queue.push(ev);


                pl->set_mp(pl->get_mp() - 1000);
                send_status_change_packet(pl);

                int taget = packet->target;// -9615;


                players[taget]->set_mp(players[taget]->get_mp() + players[taget]->get_maxmp() / 10);
                send_status_change_packet(reinterpret_cast<Player*>(players[taget]));

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
                ev.target_id = 1; //packet->target;
                timer_queue.push(ev);

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
                ev.target_id = 2;// packet->target;
                timer_queue.push(ev);

                pl->set_mp(pl->get_mp() - 1000);
                send_status_change_packet(pl);

                int taget = packet->target;// - 9615;

                players[taget]->set_hp(players[taget]->get_hp() + players[taget]->get_maxhp() / 10);
                send_status_change_packet(reinterpret_cast<Player*>(players[taget]));
                send_buff_ui_packet(reinterpret_cast<Player*>(players[taget]), 2);
                break;

            }

            break;
        }
        break;
    }
    case CS_PACKET_PARTY_ROOM: {
        // 현재 활성화 되었는 던전의 정보들을 보낸다
        for (auto& dun : dungeons) {
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

        for (auto& dun : dungeons) {
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
        send_party_room_info_packet(pl, dungeons[r_id]->get_party_palyer(),
            dungeons[r_id]->player_cnt, dungeons[r_id]->get_dungeon_id());
        break;
    }
    case CS_PACKET_RAID_RANDER_OK: {
        dungeons[pl->indun_id]->player_rander_ok++;

        if (dungeons[pl->indun_id]->player_rander_ok == GAIA_ROOM - dungeons[pl->indun_id]->partner_cnt) {
            dungeons[pl->indun_id]->start_game = true;

            // Ai움직이기 시작
            Player** party_players = dungeons[pl->indun_id]->get_party_palyer();

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
            ev.obj_id = dungeons[pl->indun_id]->get_dungeon_id();
            ev.start_time = chrono::system_clock::now() + 1s;
            ev.ev = EVENT_BOSS_MOVE;
            ev.target_id = -1;
            timer_queue.push(ev);

            ZeroMemory(&ev, sizeof(ev));
            ev.obj_id = dungeons[pl->indun_id]->get_dungeon_id();
            ev.start_time = chrono::system_clock::now() + 3s;
            ev.ev = EVENT_BOSS_ATTACK;
            ev.target_id = -1;
            timer_queue.push(ev);

            for (int i = 0; i < GAIA_ROOM; i++) {
                if (party_players[i]->get_tribe() == PARTNER) {
                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_PARTNER_MOVE;
                    ev.target_id = 1;
                    timer_queue.push(ev);

                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 1s;
                    ev.ev = EVENT_PARTNER_NORMAL_ATTACK;
                    ev.target_id = 1;
                    timer_queue.push(ev);

                    ev.obj_id = party_players[i]->get_id();
                    ev.start_time = chrono::system_clock::now() + 3s;
                    ev.ev = EVENT_PARTNER_SKILL;
                    ev.target_id = 1;
                    timer_queue.push(ev);
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

        Gaia* dun = dungeons[r_id];

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
        for (auto& dun : dungeons) {
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
        Gaia* dun = dungeons[r_id];
        dun->quit_palyer(pl);
        // 나갔다는 정보를 player에게 보내준다
        send_party_room_quit_ok_packet(pl);
        pl->join_dungeon_room = false;

        Player** party_players = dun->get_party_palyer();
        if (dun->player_cnt - dun->partner_cnt == 0) {
            // 아무도 없다는 뜻
            for (auto& pls : players) {
                if (true == is_npc(pls->get_id())) break;
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
                delete players[delete_id];
                players[delete_id] = new Player(delete_id);
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
                    continue;
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
            Gaia* dun = dungeons[r_id];

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
            for (auto& duns : dungeons) {
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
        Gaia* dun = dungeons[r_id];

        if (dun->player_cnt < GAIA_ROOM) {  // 제한 인원수 보다 적을 때만 추가 가능하도록 하자 

            int new_id = get_new_id();
            if (-1 == new_id) {
                cout << "Maxmum user overflow.Accept aborted.\n";
            }
            delete players[new_id];
            players[new_id] = new Partner(new_id);
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
            partner->set_maxmp(10000);
            partner->set_maxhp(10000);
            partner->set_hp(500);
            partner->set_mp(8000);
            partner->set_job(static_cast<JOB>(packet->job));
            partner->set_lv(25);
            partner->set_physical_attack(0.3 * 25 * 25 + 10 * 25);
            partner->set_magical_attack(0.1 * 25 * 25 + 5 * 25);
            partner->set_physical_defence(0.24 * 25 * 25 + 10 * 25);
            partner->set_magical_defence(0.17 * 25 * 25 + 10 * 25);
            partner->set_basic_attack_factor(50.0f);
            partner->set_defence_factor(0.0002);
            partner->set_element(E_WATER);
            //  여기까지 클라에서 패킷 받으면, 새 player id 생성 후 정보 초기화  

            // join dungeon party
            // 이 방에 이 플레이어를 집어 넣는다
            dun->partner_cnt++;
            dun->join_player(reinterpret_cast<Player*>(players[new_id]));

            // 이 방에 대한 정보를 보내준다
            Player** party_players = dun->get_party_palyer();
            for (int i = 0; i < dun->player_cnt; i++) {
                if (party_players[i]->get_tribe() == HUMAN)
                    send_party_room_info_packet(party_players[i], dun->get_party_palyer(), dun->player_cnt, dun->get_dungeon_id());
            }
        }
        else
            break;
    }
    default:
        break;
    }
}