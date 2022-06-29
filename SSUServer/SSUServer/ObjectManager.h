#pragma once
#include "protocol.h"
#include "Partner.h"
#include "SectorManager.h"
#include "SocketManager.h"
#include <array>

class PacketManager;

class ObjectManager
{
public:
	array <Npc*, MAX_USER + MAX_NPC> players;
	array <Obstacle*, MAX_OBSTACLE> obstacles;
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

	void error_display(int err_no);
	void Disconnect(int c_id);
	void worker();

	void set_packetManager(PacketManager* packetManager);

	int get_new_id();
	bool is_npc(int id) { return (id >= NPC_ID_START) && (id <= NPC_ID_END);}
	bool is_near(int a, int b);
};

