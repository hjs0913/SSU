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

	

public:
	SectorManager();
	void player_accept(Npc* p);
	void player_move(Npc* p);
	void player_disconnect(int player_id);
	int check_sector_id(int p_x, int p_z);
};

