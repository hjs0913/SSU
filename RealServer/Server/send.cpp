#include "send.h"

void send_login_ok_packet(Player* pl)
{
    sc_packet_login_ok packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_LOGIN_OK;
    packet.id = pl->get_Id();
    strcpy_s(packet.name, pl->get_name());
    packet.x = pl->get_x();
    packet.y = pl->get_y();
    packet.level = pl->get_lv();
    packet.hp = pl->get_hp();
    packet.maxhp = pl->get_maxhp();
    packet.exp = pl->get_exp();
    packet.tribe = pl->get_tribe();
    pl->do_send(sizeof(packet), &packet);
}

bool send_move_packet(Player* pl, Npc* mover)
{
    sc_packet_move packet;
    packet.id = mover->get_Id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_MOVE;
    packet.x = mover->get_x();
    packet.y = mover->get_y();
    packet.move_time =  pl->last_move_time;
    //cout <<"move_time" <<  pl->last_move_time << endl;
    bool error = pl->do_send(sizeof(packet), &packet);
    return error;
}

bool send_put_object_packet(Player* pl, Npc* target)
{
    sc_packet_put_object packet;
    packet.id = target->get_Id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_PUT_OBJECT;
    packet.x = target->get_x();
    packet.y = target->get_y();
    strcpy_s(packet.name, target->get_name());
    packet.object_type = target->get_tribe();
    bool error = pl->do_send(sizeof(packet), &packet);
    return error;
}

bool send_remove_object_packet(Player* pl, Npc* victim)
{
    sc_packet_remove_object packet;
    packet.id = victim->get_Id();
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_REMOVE_OBJECT;
    packet.object_type = victim->get_tribe();
    bool error = pl->do_send(sizeof(packet), &packet);
    return error;
}

bool send_chat_packet(Player* pl, int my_id, char* mess)
{
    cout << pl->get_name() << endl;
    cout << mess << endl;
    sc_packet_chat packet;
    packet.id = my_id;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_CHAT;
    strcpy_s(packet.message, mess);
    bool error = pl->do_send(sizeof(packet), &packet);
    return error;
}

void send_login_fail_packet(Player* pl, int reason)
{
    sc_packet_login_fail packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_LOGIN_FAIL;
    packet.reason = reason;
    pl->do_send(sizeof(packet), &packet);
}

bool send_status_change_packet(Player* pl)
{
    sc_packet_status_change packet;
    packet.size = sizeof(packet);
    packet.type = SC_PACKET_STATUS_CHANGE;
    packet.level = pl->get_lv();
    packet.hp = pl->get_hp();
    packet.maxhp = pl->get_maxhp();
    packet.exp = pl->get_exp();
    bool error = pl->do_send(sizeof(packet), &packet);
    return error;
}