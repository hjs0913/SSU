#pragma once
#include "Sector.h"
#include "stdafx.h"
#include "Player.h"
class SectorManager
{
private:
	const int MAX_SECTOR = 64;
	const int SECTOR_WIDTH = 510;
	const int SECTOR_HEIGHT = 510;

	array<Sector*, 64> sectors;
	array<Sector*, 64> obs_sectors;

	array <Npc*, MAX_USER + MAX_NPC> players;

public:
	SectorManager();
	
	void set_players_object(array <Npc*, MAX_USER + MAX_NPC>& pls);

	void player_put(Npc* p);
	void player_move(Npc* p);
	void player_erase(Npc* p);
	int check_sector_id(int p_x, int p_z);

	void send_put_packet_sector_player(Npc* p, int sector_id);
	void send_delete_packet_sector_player(Npc* p, int sector_id);
};

