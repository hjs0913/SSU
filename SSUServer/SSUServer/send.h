#pragma once
#include "Player.h"
#include "stdafx.h"
void send_login_ok_packet(Player* pl);

void send_move_packet(Player* pl, Npc* mover, int right);

void send_put_object_packet(Player* pl, Npc* target);

void send_remove_object_packet(Player* pl, Npc* victim);

void send_chat_packet(Player* pl, int my_id, char* mess);

void send_login_fail_packet(Player* pl, int reason);

void send_status_change_packet(Player* pl);

void send_dead_packet(Player* pl, Npc* attacker, Npc* victim);

void send_revive_packet(Player* pl, Npc* reviver);

void send_look_packet(Player* pl, Npc* changer);

void send_change_hp_packet(Player* pl, Npc* victim, float fDamage);

void send_change_mp_packet(Player* pl, Npc* victim);

void send_play_shoot_packet(Player* pl);

void send_play_effect_packet(Player* pl, Npc* npc);

void send_start_gaia_packet(Player* pl, int* id);

void send_gaia_pattern_one_packet(Player *pl, pos* pt_pos);

void send_gaia_pattern_two_packet(Player* pl, pos* pt_pos, int pattern_two_number);

void send_gaia_pattern_five_packet(Player* pl, pos* pt_pos);

void send_gaia_pattern_finish_packet(Player* pl, int pattern);

void send_change_death_count_packet(Player* pl, int dc);

void send_gaia_join_ok(Player* pl, int room_number);

void send_buff_ui_packet(Player* pl, int num);

void send_party_room_packet(Player* pl, char* room_name, int room_id);

void send_party_room_info_packet(Player* pl, Player** room_pl, int players_num, int room_id);

void send_party_room_enter_ok_packet(Player* pl, int room_id);

void send_party_room_enter_failed_packet(Player* pl, int room_id, int failed_reason);

void send_party_room_quit_ok_packet(Player* pl);

void send_party_invitation(Player* pl, int r_id, int user_id);

void send_party_invitation_failed(Player* pl, int failed_reason, char* invited_user);

void send_party_room_destroy(Player* pl, int r_id);

void send_notice(Player* pl, const char* notice_str, int raid_notice);

void send_move_openwrold(Player* pl);

void send_animation_attack(Player* pl, int id);

void send_animation_skill(Player* pl, int id, int animation_skill);
