#pragma once
const int SERVERPORT = 9000;
const int BUFSIZE = 512;

const int WORLD_WIDTH = 8;
const int WORLD_HEIGHT = 8;

const int MAX_USER = 10;
const int MAX_ID_LEN = 20;

enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT };
enum BUF_TYPE { B_NONE, B_PHYATTACK, B_MAGATTACK, B_PHYDEFENCE, B_MAGDEFENCE, B_SPEED };
enum ELEMENT{E_WATER, E_FULLMETAL, E_WIND, E_FIRE, E_TREE, E_EARTH, E_ICE};
enum TRIBE{T_HUMAN, T_MONSTER};

// 패킷 타입 정리
const int CS_PACKET_LOGIN = 1;
const int CS_PACKET_MOVE = 2;
const int CS_PACKET_ATTACK = 3;
const int CS_PACKET_DIED = 4;

const int SC_PACKET_LOGIN = 1;
const int SC_PACKET_MOVE = 2;
const int SC_PACKET_LOGOUT = 3;
const int SC_PACKET_PUT_OBJECT = 4;
const int SC_PACKET_ATTACK = 5;
const int SC_PACKET_DIED = 6;

// 패킷 정리
#pragma pack(push, 1)
struct cs_packet_login 
{
	char size;
	char type;
	char name[MAX_ID_LEN];
};

struct cs_packet_move
{
	char size;
	char type;
	char direction;
};

struct cs_packet_attack
{
	char size;
	char type;
	int skill;
};

struct sc_packet_login
{
	char size;
	char type;
	int id;
	int x, y;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int exp;
	short attack_factor;
	float defense_factor;
	TRIBE tribe;
};

struct sc_packet_move
{
	char size;
	char type;
	int id;
	int x, y;
};

struct sc_packet_logout
{
	char size;
	char type;
	int id;
	TRIBE tribe;
};

struct sc_packet_put_object
{
	char size;
	char type;
	char name[MAX_ID_LEN];
	int id;
	int x, y;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int exp;
	short attack_factor;
	float defense_factor;
	TRIBE tribe;
};

struct sc_packet_attack
{
	char size;
	char type;
	int id;
	int damage_size;
	int p_hp;
	int m_hp;
	TRIBE subject;
};

struct sc_packet_died
{
	char size;
	char type;
	int id;
};
#pragma pack(pop)