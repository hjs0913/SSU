#pragma once

// --------------------------------------------------------
// 개인 추가
enum STATE { ST_FREE, ST_ACCEPT, ST_INGAME, ST_DEAD };
enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT, OP_NPC_MOVE,
	OP_NPC_ATTACK, OP_AUTO_PLAYER_HP, OP_PLAYER_REVIVE, OP_NPC_REVIVE,
	OP_PLAYER_ATTACK, OP_NPC_AGRO, OP_ELEMENT_COOLTIME
};
enum EVENT_TYPE {
	EVENT_NPC_MOVE, EVENT_NPC_ATTACK, EVENT_AUTO_PLAYER_HP,
	EVENT_PLAYER_REVIVE, EVENT_NPC_REVIVE, EVENT_PLAYER_ATTACK,
	EVENT_SKILL_COOLTIME, EVENT_NPC_AGRO, EVENT_ELEMENT_COOLTIME
};
enum TRIBE { HUMAN, MONSTER, AGRO, BOSS, OBSTACLE };
enum BUF_TYPE { B_NONE, B_PHYATTACK, B_MAGATTACK, B_PHYDEFENCE, 
	B_MAGDEFENCE, B_SPEED, B_BURN };
enum ELEMENT { E_NONE = 0, E_WATER, E_FULLMETAL, E_WIND, E_FIRE, E_TREE, E_EARTH, E_ICE = 7};
enum JOB { J_DILLER = 0, J_TANKER, J_MAGISIAN, J_SUPPORTER = 3 };
enum MONSTER_SPECIES{FALLEN_FLOG, FALLEN_CHICKEN, FALLEN_RABBIT, 
	FALLEN_MONKEY, WOLF_BOSS, FALLEN_TIGER};

const int BUFSIZE = 256;
const int RANGE = 600;
const int AGRORANGE = 5;
const int MAX_OBSTACLE = 609;
const float PLAYER_VELOCITY = 0.5f;
//------------------------------------------------------------

const short SERVER_PORT = 9000;

const int  WORLD_HEIGHT = 4000;
const int  WORLD_WIDTH = 4000;
const int  MAX_NAME_SIZE = 20;
const int  MAX_CHAT_SIZE = 100;
const int  MAX_USER = 1000;
// const int  MAX_NPC = 200000;
const int  MAX_NPC = 180;		// 디버깅 용
constexpr int NPC_ID_START = MAX_USER;
constexpr int NPC_ID_END = MAX_USER + MAX_NPC - 1;

const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_MOVE = 2;
const char CS_PACKET_ATTACK = 3;
const char CS_PACKET_CHAT = 4;
const char CS_PACKET_TELEPORT = 5;
const char CS_PACKET_SKILL = 6;
const char CS_PACKET_LOOK = 7;
const char CS_PACKET_CHANGE_JOB = 8;
const char CS_PACKET_CHANGE_ELEMENT = 9;


const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_MOVE = 2;
const char SC_PACKET_PUT_OBJECT = 3;
const char SC_PACKET_REMOVE_OBJECT = 4;
const char SC_PACKET_CHAT = 5;
const char SC_PACKET_LOGIN_FAIL = 6;
const char SC_PACKET_STATUS_CHANGE = 7;
const char SC_PACKET_DEAD = 8;
const char SC_PACKET_REVIVE = 9;
const char SC_PACKET_LOOK = 10;
const char SC_PACKET_CHANGE_JOB = 11;
const char SC_PACKET_CHANGE_ELEMENT = 12;
const char SC_PACKET_CHANGE_HP = 13;
//---------------------------------------------------
#pragma pack (push, 1)
struct cs_packet_login {
	unsigned char size;
	char	type;
	char	name[MAX_NAME_SIZE];
};

struct cs_packet_move {
	unsigned char size;
	char	type;
	//char	direction;			// 0 : up,  1: down, 2:left, 3:right
	float	x, y, z;
	int		move_time;
};

struct cs_packet_attack {
	unsigned char size;
	char	type;
};

struct cs_packet_chat {
	unsigned char size;
	char	type;
	char	message[MAX_CHAT_SIZE];
};

struct cs_packet_teleport { 
	// 서버에서 장애물이 없는 랜덤 좌표로 텔레포트 시킨다.
	// 더미 클라이언트에서 동접 테스트용으로 사용.
	unsigned char size;
	char	type;
};

struct cs_packet_skill {
	unsigned char size;
	char type;
	char job;
	char skill_type;    //0 : 물리 공격 1: 마법 공격  2 : 버프 
	char skill_num;    // 0-0, 0-1   ,,,,,   1-0,  1-1  
};

struct cs_packet_look {
	unsigned char size;
	char type;
	float x, y, z;	// look
	float right_x, right_y, right_z;	// right
};

struct cs_packet_change_job {
	unsigned char size;
	char type;
	JOB job;
};

struct cs_packet_change_element {
	unsigned char size;
	char type;
	ELEMENT element;
};

struct sc_packet_login_ok {
	unsigned char size;
	char type;
	int		id;
	char	name[MAX_NAME_SIZE];	// 기존 프로토콜에 없어서 추가해주었습니다
	float	x, y, z;
	short	level;
	int		hp, maxhp;
	int		mp, maxmp;
	int		exp;
	short	tribe;					// 기존 프로토콜에 없어서 추가해주었습니다
	JOB job;
	ELEMENT element;
};

struct sc_packet_move {
	unsigned char size;
	char type;
	int		id;
	float  x, y, z;
	int		move_time;
};

struct sc_packet_put_object {
	unsigned char size;
	char type;
	int id;
	float x, y, z;
	float look_x, look_y, look_z;
	short	level;
	int		hp, maxhp;
	int		mp, maxmp;
	ELEMENT element;
	char	object_type;
	char	object_class;
	char	name[MAX_NAME_SIZE];
};

struct sc_packet_remove_object {
	unsigned char size;
	char type;
	int id;
	char object_type;	// 새로 추가해줌
};

struct sc_packet_chat {
	unsigned char size;
	char type;
	int id;
	char message[MAX_CHAT_SIZE];
};

struct sc_packet_login_fail {
	unsigned char size;
	char type;
	int	 reason;		// 0: 중복 ID,  1:사용자 Full
};

struct sc_packet_status_change {
	unsigned char size;
	char	type;         
	int		id;
	short	level;
	int		hp, maxhp, mp, maxmp;
	int		exp;
	JOB job;
	ELEMENT element;
};

struct sc_packet_dead {
	unsigned char size;
	char type;
	int id;
	int attacker_id;
};

struct sc_packet_revive {
	unsigned char size;
	char type;
	int id;
	float	x, y, z;
	int		hp;
	int		exp;
};

struct sc_packet_look {
	unsigned char size;
	char type;
	int id;
	float x, y, z;	// look
};

struct sc_packet_change_hp {
	unsigned char size;
	char type;
	int id;
	int hp;
};

#pragma pack(pop)