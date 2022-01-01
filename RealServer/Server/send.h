#pragma once
#include "Player.h"
#include "stdafx.h"
void send_login_ok_packet(Player* pl);

bool send_move_packet(Player* pl, Npc* mover);

bool send_put_object_packet(Player* pl, Npc* target);

bool send_remove_object_packet(Player* pl, Npc* victim);

bool send_chat_packet(Player* pl, int my_id, char* mess);

void send_login_fail_packet(Player* pl, int reason);

bool send_status_change_packet(Player* pl);