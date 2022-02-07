#pragma once
#include "Obstacle.h"
class Npc : public Obstacle
{
private:
	MONSTER_SPECIES _mon_species;

protected:
	char	_name[MAX_NAME_SIZE];
	int		_hp;	
	int		_maxhp;
	short	_lv;
	float	_physical_attack;
	float	_magical_attack;
	float	_physical_defence;
	float	_magical_defence;
	float	_basic_attack_factor;
	float	_defence_factor;
	ELEMENT	_element;
	float	_move_speed;
	float	_attack_speed;
	float	_look_x, _look_y, _look_z;
	float	_right_x, _right_y, _right_z;

	STATE	_state;
	atomic_bool	_active;		// NPC가 가만히 안있고 움직일때

	//skill
	float _skill_factors[3][10];

public:
	mutex	state_lock;
	mutex	lua_lock;
	lua_State* L;

	Npc(int id);
	Npc(int id, const char* name);
	~Npc();

	void set_pos(int x, int y);
	void set_state(STATE s);
	void set_name(const char* name);

	void set_active(bool act);
	void set_tribe(TRIBE tribe);
	void set_lv(short lv);
	void set_hp(int hp);
	void set_maxhp(int m_hp);
	void set_physical_attack(float physical_attack);
	void set_magical_attack(float magical_attack);
	void set_physical_defence(float physical_defence);
	void set_magical_defence(float magical_defence);
	void set_basic_attack_factor(float basic_attack);
	void set_defence_factor(float defence_factor);
	void set_look(float look_x, float look_y, float look_z);
	void set_right(float right_x, float right_y, float right_z);	// _look_x, y, z로 만들자
	void set_mon_species(int s);

	char* get_name();
	STATE get_state();
	bool get_active();
	TRIBE get_tribe();
	short get_lv();
	int get_hp();
	int get_maxhp();
	float get_physical_attack();
	float get_magical_attack();
	float get_physical_defence();
	float get_magical_defence();
	float get_basic_attack_factor();
	float get_defence_factor();
	float get_look_x();
	float get_look_y();
	float get_look_z();
	float get_right_x();
	float get_right_y();
	float get_right_z();
	MONSTER_SPECIES get_mon_spices();


	//skill
	void set_skill_factor(int skill_type, int skill_num);
	float get_skill_factor(int skill_type, int skill_num);

	
};

