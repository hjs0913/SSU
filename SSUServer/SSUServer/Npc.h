#pragma once
#include "Obstacle.h"
class Npc : public Obstacle
{
private:
	MONSTER_SPECIES _mon_species;
	int				_target_id;
	lua_State*		L;
	mutex	lua_lock;
protected:
	char	_name[MAX_NAME_SIZE];
	int		_hp;
	int		_maxhp;
	int		_mp;
	int		_maxmp;
	short	_lv;
	float	_physical_attack;
	float	_magical_attack;
	float	_physical_defence;
	float	_magical_defence;

	float	_origin_physical_attack;
	float	_origin_magical_attack;
	float	_origin_physical_defence;
	float	_origin_magical_defence;

	float	_basic_attack_factor;
	float	_defence_factor;
	ELEMENT	_element;
	float	_move_speed;
	float	_attack_speed;
	float	_look_x, _look_y, _look_z;
	float	_right_x, _right_y, _right_z;
	bool superposition;
	STATE	_state;
	atomic_bool	_active;		// NPC가 공격하는지 아닌지 판단
	atomic_bool _move_active;	// NPC가 움직이는지 아닌지 판단

	//skill
	float _skill_factors[3][10];
	
	bool burn_on;

public:
	mutex	state_lock;
	mutex	sector_lock;

	mutex		        vl;
	unordered_set<int>	viewlist;

	Npc(int id);
	Npc(int id, const char* name);
	~Npc();

	void Initialize_Lua(const char* f_lua);
	void Initialize_Lua_Boss(const char* f_lua, int dungeon_id);
	void Initialize_Boss(int dungeon_id);

	void set_pos(int x, int z);
	void set_state(STATE s);
	void set_name(const char* name);

	void set_active(bool act);
	void set_move_active(bool mv_act);
	void set_tribe(TRIBE tribe);
	void set_lv(short lv);
	void set_hp(int hp);
	void set_maxhp(int m_hp);
	void set_mp(int hp);
	void set_maxmp(int m_hp);
	void set_physical_attack(float physical_attack);
	void set_magical_attack(float magical_attack);
	void set_physical_defence(float physical_defence);
	void set_magical_defence(float magical_defence);
	void set_basic_attack_factor(float basic_attack);
	void set_defence_factor(float defence_factor);
	void set_look(float look_x, float look_y, float look_z);
	void set_right(float right_x, float right_y, float right_z);	// _look_x, y, z로 만들자
	void set_mon_species(int s);

	void set_element(ELEMENT element);
	ELEMENT get_element();

	void set_target_id(int target);
	int get_target_id();

	char* get_name();
	STATE get_state();
	bool get_active();
	bool get_move_active();
	TRIBE get_tribe();
	short get_lv();
	int get_hp();
	int get_maxhp();
	int get_mp();
	int get_maxmp();
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

	//origin
	void set_origin_physical_attack(float physical_attack);
	void set_origin_magical_attack(float magical_attack);
	void set_origin_physical_defence(float physical_defence);
	void set_origin_magical_defence(float magical_defence);

	float get_origin_physical_attack();
	float get_origin_magical_attack();
	float get_origin_physical_defence();
	float get_origin_magical_defence();

	//skill
	void set_skill_factor(int skill_type, int skill_num);
	float get_skill_factor(int skill_type, int skill_num);

	void set_element_cooltime(bool yn);
	bool get_element_cooltime();

	// move
	bool check_move_alright(int x, int z, bool monster, const array<Obstacle*, MAX_OBSTACLE>& obstacles);
	pos non_a_star(int t_x, int t_z, int x, int z);
	pos a_star(int t_x, int t_z, int x, int z, const array<Obstacle*, MAX_OBSTACLE>& obstacles);
	int huristic(int t_x, int t_z, int x, int z);
	void push_npc_move_event();
	void return_npc_position(const array<Obstacle*, MAX_OBSTACLE> &obstacles);
	void do_npc_move(Npc* target, const array<Obstacle*, MAX_OBSTACLE>& obstacles);
	void npc_roming(const array<Obstacle*, MAX_OBSTACLE>& obstacles);

	// attack
	bool npc_attack_validation(Npc* target);
	virtual void attack_dead_judge(Npc* target);	// 죽었는지 아닌지 판정
	virtual void attack_element_judge(Npc* target);	// 공격에 대한 속성 판정
	virtual void basic_attack_success(Npc* target);	// 일반공격 데미지 계산
	virtual void phisical_skill_success(Npc* target, float skill_factor);	// 물리스킬 데미지 계산
	virtual void magical_skill_success(Npc* target, float skill_factor);	// 마법스킬 데미지 계산


	virtual void revive();
};