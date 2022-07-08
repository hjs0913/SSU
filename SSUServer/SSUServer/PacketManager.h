#pragma once
#include "ObjectManager.h"
class PacketManager
{
private:
	ObjectManager* m_ObjectManger;
	SectorManager* m_SectorManager;
	array <Npc*, MAX_USER + MAX_NPC + MAX_AI> players;
	HANDLE* h_iocp;

public:
	PacketManager(ObjectManager* objectManager, SectorManager* sectorManager, HANDLE* iocp);
	void set_players_object(array <Npc*, MAX_USER + MAX_NPC + MAX_AI>& pls);
	void process_packet(Player* pl, unsigned char* p);

	void skill_cooltime(int client_id, chrono::system_clock::time_point t , int skill_id);
};

