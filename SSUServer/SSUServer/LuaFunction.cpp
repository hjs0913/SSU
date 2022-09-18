#include "LuaFunction.h"
#include "ObjectManager.h"

int API_get_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = 0;
	if(static_ObjectManager::get_objManger() != nullptr)
		x = static_ObjectManager::get_player(user_id)->get_x();
	//int x = players[user_id]->get_x();
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		y = static_ObjectManager::get_player(user_id)->get_y();
	lua_pushnumber(L, y);
	return 1;
}

int API_get_z(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int z = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		z = static_ObjectManager::get_player(user_id)->get_z();
	lua_pushnumber(L, z);
	return 1;
}

int API_get_look_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		x = static_ObjectManager::get_player(user_id)->get_look_x();
	//int x = players[user_id]->get_x();
	lua_pushnumber(L, x);
	return 1;
}

int API_get_look_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		y = static_ObjectManager::get_player(user_id)->get_look_y();
	lua_pushnumber(L, y);
	return 1;
}

int API_get_look_z(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int z = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		z = static_ObjectManager::get_player(user_id)->get_look_z();
	lua_pushnumber(L, z);
	return 1;
}

int API_get_right_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		x = static_ObjectManager::get_player(user_id)->get_right_x();
	//int x = players[user_id]->get_x();
	lua_pushnumber(L, x);
	return 1;
}

int API_get_right_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		y = static_ObjectManager::get_player(user_id)->get_right_y();
	lua_pushnumber(L, y);
	return 1;
}

int API_get_right_z(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int z = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		z = static_ObjectManager::get_player(user_id)->get_right_z();
	lua_pushnumber(L, z);
	return 1;
}

int API_get_hp(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int hp = 0;
	if (static_ObjectManager::get_objManger() != nullptr)
		hp = static_ObjectManager::get_player(user_id)->get_hp();
	//lua_pushnumber(L, hp);
	lua_pushinteger(L, hp);
	return 1;
}
