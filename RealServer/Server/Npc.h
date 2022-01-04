#pragma once
#include "Obstacle.h"
class Npc : public Obstacle
{
protected:
	char	_name[MAX_NAME_SIZE];
	int		_hp;	
	int		_maxhp;
	short	_lv;
	float	_physical_attack;
	float	_magical_attack;
	float	_physical_defence;
	float	_magical_defence;
	ELEMENT	_element;
	float	_move_speed;
	float	_attack_speed;

	STATE	_state;
	atomic_bool	_active;		// NPC가 가만히 안있고 움직일때


public:
	mutex	state_lock;
	mutex	lua_lock;
	lua_State* L;

	Npc(int id);
	Npc(int id, const char* name);
	~Npc();
	


	void set_initialize();

	void set_pos(int x, int y);
	void set_x(int x);
	void set_y(int y);
	void set_state(STATE s);
	void set_name(const char* name);
	void set_id(int id);
	void set_active(bool act);
	void set_tribe(TRIBE tribe);
	void set_lv(short lv);
	void set_hp(int hp);
	void set_maxhp(int m_hp);
	void set_physical_attack(float physical_attack);
	void set_magical_attack(float magical_attack);
	void set_physical_defence(float physical_defence);
	void set_magical_defence(float magical_defence);

	int get_x();
	int get_y();
	int get_Id();
	char* get_name();
	STATE get_state();
	bool get_active();
	TRIBE get_tribe();
	short get_lv();
	int get_hp();
	int get_maxhp();
};

