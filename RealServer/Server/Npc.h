#pragma once
#include "Obstacle.h"
class Npc : public Obstacle
{
protected:
	char	_name[MAX_NAME_SIZE];
	int		_hp;	
	int		_maxhp;
	short	_lv;	
	int		_exp;	

	STATE	_state;
	atomic_bool	_active;		// NPC가 가만히 안있고 움직일때


public:
	mutex	state_lock;
	mutex	lua_lock;
	lua_State* L;

	Npc(int id);
	Npc(int id, const char* name);
	~Npc();
	
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
	void set_exp(int exp);
	void set_maxhp(int m_hp);

	int get_x();
	int get_y();
	int get_Id();
	char* get_name();
	STATE get_state();
	bool get_active();
	TRIBE get_tribe();
	short get_lv();
	int get_hp();
	int get_exp();
	int get_maxhp();
};

