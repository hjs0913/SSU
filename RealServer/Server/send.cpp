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
    packet.exp = pl->get_exp();
    packet.tribe = pl->get_tribe();
    pl->do_send(sizeof(packet), &packet);
}

void send_move_packet(Player* pl, Npc* mover)
{
    sc_packet_move packet;
    packet.id = mover->get_id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_MOVE;
    packet.x = mover->get_x();
    packet.y = mover->get_y();
    packet.z = mover->get_z();
    packet.move_time =  pl->last_move_time;
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
    packet.object_type = target->get_tribe();
    if (target->get_tribe() == HUMAN) {
        packet.object_class = reinterpret_cast<Player*>(target)->get_job();
    }
    else if (target->get_tribe() == MONSTER) {
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
    cout << pl->get_name() << endl;
    cout << mess << endl;
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
    packet.level = pl->get_lv();
    packet.hp = pl->get_hp();
    packet.maxhp = pl->get_maxhp();
    packet.mp = pl->get_mp();
    packet.maxmp = pl->get_maxmp();
    packet.exp = pl->get_exp();
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