#pragma once
#include "protocol.h"
#include "Partner.h"
#include "SectorManager.h"
#include "SocketManager.h"
#include <array>

class PacketManager;

class ObjectManager
{
private:
	array <Npc*, MAX_USER + MAX_NPC + MAX_AI> players;
	array <Obstacle*, MAX_OBSTACLE> obstacles;
	array <Obstacle*, 48> house_obstacles;
	array <Gaia*, MAX_DUNGEONS> dungeons;

	SectorManager* m_SectorManager;
	PacketManager* m_PacketManager;
	SOCKET* s_socket;
	HANDLE* h_iocp;

public:
	ObjectManager(SectorManager* sectorManager, MainSocketManager* socket);
	void Initialize_Npc();
	void Initialize_Obstacle();
	void Initialize_Dungeons();
	void Initialize_Ai();

	void error_display(int err_no);
	void Disconnect(int c_id);
	void worker();

	void set_packetManager(PacketManager* packetManager);

	int get_new_id();
	int get_new_ai_id();
	bool is_npc(int id) { return (id >= NPC_ID_START) && (id <= NPC_ID_END);}
	bool is_near(int a, int b);
	bool check_move_alright(int x, int z, bool monster);
	bool check_move_alright_indun(int x, int z);

	Npc* get_player(int c_id);
	Gaia* get_dungeon(int d_id);
	array<Gaia*, MAX_DUNGEONS>& get_dungeons();

	void CloseServer();
};

class static_ObjectManager
{
private:
	static ObjectManager* objManager;

public:
	static_ObjectManager() {};
	~static_ObjectManager() {};

	static void* set_objManger(ObjectManager* om);
	static ObjectManager* get_objManger();

	static Npc* get_player(int c_id);
};