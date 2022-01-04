#pragma once

// --------------------------------------------------------
// 개인 추가
enum STATE { ST_FREE, ST_ACCEPT, ST_INGAME, ST_DEAD };
enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT, OP_NPC_MOVE,
	OP_NPC_ATTACK, OP_AUTO_PLAYER_HP, OP_PLAYER_REVIVE, OP_NPC_REVIVE,
	OP_PLAYER_ATTACK};
enum EVENT_TYPE { EVENT_NPC_MOVE, EVENT_NPC_ATTACK, EVENT_AUTO_PLAYER_HP,
	EVENT_PLAYER_REVIVE, EVENT_NPC_REVIVE, EVENT_PLAYER_ATTACK,
	EVENT_SKILL_COOLTIME};
enum TRIBE { HUMAN, MONSTER, AGRO, BOSS, OBSTACLE };
enum BUF_TYPE { B_NONE, B_PHYATTACK, B_MAGATTACK, B_PHYDEFENCE, 
	B_MAGDEFENCE, B_SPEED, B_BURN };
enum ELEMENT { E_NONE, E_WATER, E_FULLMETAL, E_WIND, E_FIRE, E_TREE, E_EARTH, E_ICE };

const int BUFSIZE = 256;
const int RANGE = 7;
const int AGRORANGE = 5;
const int MAX_OBSTACLE = 10000;
//------------------------------------------------------------

const short SERVER_PORT = 9000;

const int  WORLD_HEIGHT = 2000;
const int  WORLD_WIDTH = 2000;
const int  MAX_NAME_SIZE = 20;
const int  MAX_CHAT_SIZE = 100;
const int  MAX_USER = 10000;
// const int  MAX_NPC = 200000;
const int  MAX_NPC = 10000;		// 디버깅 용
constexpr int NPC_ID_START = MAX_USER;
constexpr int NPC_ID_END = MAX_USER + MAX_NPC - 1;

const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_MOVE = 2;
const char CS_PACKET_ATTACK = 3;
const char CS_PACKET_CHAT = 4;
const char CS_PACKET_TELEPORT = 5;

const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_MOVE = 2;
const char SC_PACKET_PUT_OBJECT = 3;
const char SC_PACKET_REMOVE_OBJECT = 4;
const char SC_PACKET_CHAT = 5;
const char SC_PACKET_LOGIN_FAIL = 6;
const char SC_PACKET_STATUS_CHANGE = 7;
//---------------------------------------------------
// 추가사항
const char CS_PACKET_SKILL = 6;
const char SC_PACKET_DEAD = 8;
const char SC_PACKET_REVIVE = 9;
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
	char	direction;			// 0 : up,  1: down, 2:left, 3:right
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

struct sc_packet_login_ok {
	unsigned char size;
	char type;
	int		id;
	char	name[MAX_NAME_SIZE];	// 기존 프로토콜에 없어서 추가해주었습니다
	float	x, y, z;
	short	level;
	short	hp, maxhp;
	int		exp;
	short	tribe;					// 기존 프로토콜에 없어서 추가해주었습니다
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
	char object_type;
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
	char type;
	short	level;
	short	hp, maxhp;
	int		exp;
};

#pragma pack(pop)
// 추가 사항 -----------------------------

struct cs_packet_skill {
	unsigned char size;
	char type;
	char skill_type;	
	// 0 : 대각선까지 범위형 공격
	// 1 : 직선 쭉 뚫고 가는 공격
	// 2 : 버프형 공격(공격이 10초동안 50%상승)
};

struct sc_packet_dead {
	unsigned char size;
	char type;
	int attacker_id;
};

struct sc_packet_revive {
	unsigned char size;
	char type;
	short	x, y;
	int		hp;
	int		exp;
};