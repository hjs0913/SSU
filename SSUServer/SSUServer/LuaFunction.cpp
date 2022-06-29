#include "LuaFunction.h"

int API_get_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = players[user_id]->get_x();
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = players[user_id]->get_y();
	lua_pushnumber(L, y);
	return 1;
}

int API_get_z(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int z = players[user_id]->get_z();
	lua_pushnumber(L, z);
	return 1;
}