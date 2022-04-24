#include "send.h"
#include "Partner.h"



Partner::Partner(int d_id)
{
	dungeon_id = d_id;
	st = DUN_ST_ROBBY;
	player_cnt = 0;

	player_rander_ok = 0;
	start_game = false;
	target_id = 0;

	party_id = new int[GAIA_ROOM];
	running_pattern = false;

	// Boss Npc Intialize	
	partner = new Player(dungeon_id);
	partner->set_tribe(PARTNER);
	partner->state_lock.lock();
	partner->set_state(ST_FREE);
	partner->state_lock.unlock();
	partner->set_id(102);
	
	/*lua_State* L = boss->L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L, "Raid_Gaia.lua") || lua_pcall(L, 0, 0, 0);
	lua_getglobal(L, "set_uid");
	lua_pushnumber(L, dungeon_id);
	error = lua_pcall(L, 1, 10, 0);

	boss->set_element(static_cast<ELEMENT>(lua_tointeger(L, -10)));
	boss->set_lv(lua_tointeger(L, -9));

	boss->set_name(lua_tostring(L, -8));

	boss->set_hp(lua_tointeger(L, -7));
	boss->set_maxhp(lua_tointeger(L, -7));

	boss->set_physical_attack(lua_tonumber(L, -6));
	boss->set_magical_attack(lua_tonumber(L, -5));
	boss->set_physical_defence(lua_tonumber(L, -4));
	boss->set_magical_defence(lua_tonumber(L, -3));
	boss->set_basic_attack_factor(lua_tointeger(L, -2));
	boss->set_defence_factor(lua_tonumber(L, -1));

	lua_pop(L, 11);// eliminate set_uid from stack after call

	boss->set_x(310);
	boss->set_x(110);

	boss->set_mon_species(RAID_GAIA); 
	*/
}

Partner::~Partner()
{
	delete party;
}

void Partner::partner_move()
{
	// Raid Map은 장애물이 없으므로 A_star는 낭비다
	if (running_pattern) return;

	if ((party[target_id]->get_x() >= partner->get_x() - 8 && party[target_id]->get_x() <= partner->get_x() + 8) &&
		(party[target_id]->get_z() >= partner->get_z() - 8 && party[target_id]->get_z() <= partner->get_z() + 8)) return;

	pos mv = partner->non_a_star(party[target_id]->get_x(), party[target_id]->get_z(), partner->get_x(), partner->get_z());

	//값을 적용시키고 새로운 좌표를 클라이언트에게 보내주기
	partner->set_x(mv.first);
	partner->set_z(mv.second);
	
	for (auto pt : party) {
		send_move_packet(pt, partner);
		send_look_packet(pt, partner);
	}

}