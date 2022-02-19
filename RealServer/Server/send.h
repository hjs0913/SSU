#pragma once
#include "Player.h"
#include "stdafx.h"
void send_login_ok_packet(Player* pl);

void send_move_packet(Player* pl, Npc* mover);

void send_put_object_packet(Player* pl, Npc* target);

void send_remove_object_packet(Player* pl, Npc* victim);

void send_chat_packet(Player* pl, int my_id, char* mess);

void send_login_fail_packet(Player* pl, int reason);

void send_status_change_packet(Player* pl);

void send_look_packet(Player* pl, Npc* changer);
