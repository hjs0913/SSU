#include "send.h"

void send_login_ok_packet(Player* pl)
{
    sc_packet_login_ok packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_LOGIN_OK;
    packet.id = pl->get_id();
    strcpy_s(packet.name, pl->get_name());
    packet.x = pl->get_x();
    packet.y = pl->get_y();
    packet.z = pl->get_z();
    packet.level = pl->get_lv();
    packet.hp = pl->get_hp();
    packet.maxhp = pl->get_maxhp();
    packet.mp = pl->get_mp();
    packet.maxmp = pl->get_maxmp();
    packet.exp = pl->get_exp();
    packet.tribe = pl->get_tribe();
    packet.job = pl->get_job();
    packet.element = pl->get_element();
    pl->do_send(sizeof(packet), &packet);
}

void send_move_packet(Player* pl, Npc* mover, int right)
{
    sc_packet_move packet;
    packet.id = mover->get_id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_MOVE;
    packet.x = mover->get_x();
    packet.y = mover->get_y();
    packet.z = mover->get_z();
    packet.lx = mover->get_look_x();
    packet.ly = mover->get_look_y();
    packet.lz = mover->get_look_z();
    packet.move_time =  pl->last_move_time;
    packet.move_right = right;
    pl->do_send(sizeof(packet), &packet);
}

void send_put_object_packet(Player* pl, Npc* target)
{
    sc_packet_put_object packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PUT_OBJECT;
    packet.id = target->get_id();
    packet.x = target->get_x();
    packet.y = target->get_y();
    packet.z = target->get_z();
    packet.look_x = target->get_look_x();
    packet.look_y = target->get_look_y();
    packet.look_z = target->get_look_z();

    packet.level = target->get_lv();
    packet.hp = target->get_hp();
    packet.maxhp = target->get_maxhp();
    packet.mp = target->get_mp();
    packet.maxmp = target->get_maxmp();
    packet.element = target->get_element();

    packet.object_type = target->get_tribe();
    if (target->get_tribe() == HUMAN || target->get_tribe() == PARTNER) {
        packet.object_class = reinterpret_cast<Player*>(target)->get_job();
    }
    else if (target->get_tribe() == MONSTER || target->get_tribe() == BOSS) {
        packet.object_class = target->get_mon_spices();
    }

    strcpy_s(packet.name, target->get_name());
    pl->do_send(sizeof(packet), &packet);
}

void send_remove_object_packet(Player* pl, Npc* victim)
{
    sc_packet_remove_object packet;
    packet.id = victim->get_id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_REMOVE_OBJECT;
    packet.object_type = victim->get_tribe();
    pl->do_send(sizeof(packet), &packet);
}

void send_chat_packet(Player* pl, int my_id, char* mess)
{
    sc_packet_chat packet;
    packet.id = my_id;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_CHAT;
    strcpy_s(packet.message, mess);
    pl->do_send(sizeof(packet), &packet);
}

void send_login_fail_packet(Player* pl, int reason)
{
    sc_packet_login_fail packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_LOGIN_FAIL;
    packet.reason = reason;
    pl->do_send(sizeof(packet), &packet);
}

void send_status_change_packet(Player* pl)
{
    sc_packet_status_change packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_STATUS_CHANGE;
    packet.id = pl->get_id();
    packet.level = pl->get_lv();
    packet.hp = pl->get_hp();
    packet.maxhp = pl->get_maxhp();
    packet.mp = pl->get_mp();
    packet.maxmp = pl->get_maxmp();
    packet.exp = pl->get_exp();
    packet.job = pl->get_job();
    packet.element = pl->get_element();
    pl->do_send(sizeof(packet), &packet);
}

void send_dead_packet(Player* pl, Npc* attacker,  Npc* victim) {
    sc_packet_dead packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_DEAD;
    packet.id = victim->get_id();
    packet.attacker_id = attacker->get_id();
    pl->do_send(sizeof(packet), &packet);
}

void send_revive_packet(Player* pl, Npc* reviver)
{
    sc_packet_revive packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_REVIVE;
    packet.id = reviver->get_id();
    packet.x = reviver->get_x();
    packet.y = reviver->get_y();
    packet.z = reviver->get_z();
    packet.hp = reviver->get_hp();
    packet.exp = reinterpret_cast<Player*>(reviver)->get_exp();
    pl->do_send(sizeof(packet), &packet);
}

void send_look_packet(Player* pl, Npc* changer)
{
    sc_packet_look packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_LOOK;
    packet.id = changer->get_id();
    packet.x = changer->get_look_x();
    packet.y = changer->get_look_y();
    packet.z = changer->get_look_z();
    pl->do_send(sizeof(packet), &packet);
}

void send_change_hp_packet(Player* pl, Npc* victim, float fDamage)
{
    sc_packet_change_hp packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_CHANGE_HP;
    packet.id = victim->get_id();
    packet.hp = victim->get_hp();
    packet.damage = fDamage;
    pl->do_send(sizeof(packet), &packet);
}

void send_change_mp_packet(Player* pl, Npc* victim)
{
    sc_packet_change_mp packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_CHANGE_MP;
    packet.id = victim->get_id();
    packet.mp = victim->get_mp();
    pl->do_send(sizeof(packet), &packet);
}

void send_play_shoot_packet(Player* pl)
{
    sc_packet_play_shoot packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PLAY_SHOOT;
    pl->do_send(sizeof(packet), &packet);
}

void send_play_effect_packet(Player* pl, Npc* npc )
{
    sc_packet_play_effect packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PLAY_EFFECT;
    
    packet.hit = true;
    packet.x = npc->get_x();
    packet.y = npc->get_y();
    packet.z = npc->get_z();
    packet.id = npc->get_id();

    pl->do_send(sizeof(packet), &packet);
}

void send_start_gaia_packet(Player* pl, int* id) 
{
    sc_packet_start_gaia packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_START_GAIA;
    for (int i = 0; i < GAIA_ROOM; i++) packet.party_id[i] = id[i];
    pl->do_send(sizeof(packet), &packet);
}

void send_gaia_pattern_one_packet(Player* pl, pos* pt_pos)
{
    sc_packet_gaia_pattern_one packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_GAIA_PATTERN_ONE;

    for (int i = 0; i < 4; i++) {
        packet.point_x[i] = pt_pos[i].first;
        packet.point_z[i] = pt_pos[i].second;
    }
    pl->do_send(sizeof(packet), &packet);
}

void send_gaia_pattern_two_packet(Player* pl, pos* pt_pos, int pattern_two_number)
{
    sc_packet_gaia_pattern_two packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_GAIA_PATTERN_TWO;

    for (int i = 0; i < 3; i++) {
        packet.point_x[i] = pt_pos[i].first;
        packet.point_z[i] = pt_pos[i].second;
    }
    packet.pattern_number = pattern_two_number;
    pl->do_send(sizeof(packet), &packet);
}

void send_gaia_pattern_five_packet(Player* pl, pos* pt_pos)
{
    sc_packet_gaia_pattern_five packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_GAIA_PATTERN_FIVE;

    packet.point_x = pt_pos->first;
    packet.point_z = pt_pos->second;
    
    pl->do_send(sizeof(packet), &packet);
}

void send_gaia_pattern_finish_packet(Player* pl, int pattern)
{
    sc_packet_gaia_pattern_finish packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_GAIA_PATTERN_FINISH;
    packet.pattern = pattern;
    pl->do_send(sizeof(packet), &packet);
}

void send_change_death_count_packet(Player* pl, int dc)
{
    sc_packet_change_death_count packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_CHANGE_DEATH_COUNT;
    packet.death_count = dc;
    pl->do_send(sizeof(packet), &packet);
}

void send_gaia_join_ok(Player* pl, int room_number)
{
    sc_packet_gaia_join_ok packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_GAIA_JOIN_OK;
    packet.room_number = room_number;
    pl->do_send(sizeof(packet), &packet);
}

void send_buff_ui_packet(Player* pl, int num)
{
  
    sc_packet_buff_ui packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_BUFF_UI;
    packet.buff_num = num;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_packet(Player* pl, char* room_name, int room_id)
{
    sc_packet_party_room packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM;
    strcpy_s(packet.room_name, room_name);
    packet.room_id = room_id;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_info_packet(Player* pl, Player** room_pl, int players_num, int room_id) 
{
    sc_packet_party_room_info packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM_INFO;
    packet.room_id = room_id;
    packet.players_num = players_num;
    strcpy_s(packet.player_name1, room_pl[0]->get_name());
    
    if (players_num >= 2) {
        strcpy_s(packet.player_name2, room_pl[1]->get_name());
        if (players_num >= 3) {
            strcpy_s(packet.player_name3, room_pl[2]->get_name());
            if(players_num == 4) strcpy_s(packet.player_name4, room_pl[3]->get_name());
        }
    }

    for (int i = 0; i < players_num; i++) {
        packet.players_lv[i] = room_pl[i]->get_lv();
        packet.players_job[i] = room_pl[i]->get_job();
        packet.players_id_in_server[i] = room_pl[i]->get_id();
    }
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_enter_ok_packet(Player* pl, int room_id)
{
    sc_packet_party_room_enter_ok packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM_ENTER_OK;
    packet.room_id = room_id;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_enter_failed_packet(Player* pl, int room_id, int failed_reason)
{
    sc_packet_party_room_enter_failed packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM_ENTER_FAILED;
    packet.room_id = room_id;
    packet.failed_reason = failed_reason;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_quit_ok_packet(Player* pl)
{
    sc_packet_party_room_quit_ok packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM_QUIT_OK;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_invitation(Player* pl, int r_id, int user_id)
{
    sc_packet_party_invitation packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_INVITATION;
    packet.room_id = r_id;
    packet.invite_user_id = user_id;
    pl->do_send(sizeof(packet), &packet);
}

void send_party_invitation_failed(Player* pl, int failed_reason, char* invited_user)
{
    sc_packet_party_invitation_failed packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_INVITATION_FAILED;
    packet.failed_reason = failed_reason;
    strcpy_s(packet.invited_user, invited_user);
    pl->do_send(sizeof(packet), &packet);
}

void send_party_room_destroy(Player* pl, int r_id)
{
    sc_packet_party_room_destroy packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PARTY_ROOM_DESTROY;
    packet.room_id = r_id;
    pl->do_send(sizeof(packet), &packet);
}

void send_notice(Player* pl, const char* notice_str, int raid_notice)
{
    sc_packet_notice packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_NOTICE;
    strcpy_s(packet.message, notice_str);
    packet.raid_enter = raid_notice;
    pl->do_send(sizeof(packet), &packet);
}

void send_move_openwrold(Player* pl)
{
    sc_packet_move_openworld packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_MOVE_OPENWORLD;
    packet.x = pl->get_x();
    packet.y = pl->get_y();
    packet.z = pl->get_z();
    pl->do_send(sizeof(packet), &packet);
}

void send_animation_attack(Player* pl, int id)
{
    sc_packet_animation_attack packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_ANIMATION_ATTACK;
    packet.id = id;
    pl->do_send(sizeof(packet), &packet);
}

void send_animation_skill(Player* pl, int id, int animation_skill)
{
    sc_packet_animation_skill packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_ANIMATION_SKILL;
    packet.id = id;
    packet.animation_skill = animation_skill;
    pl->do_send(sizeof(packet), &packet);
}
