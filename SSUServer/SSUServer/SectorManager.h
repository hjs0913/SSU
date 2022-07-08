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

	array <Npc*, MAX_USER + MAX_NPC + MAX_AI> players;

public:
	SectorManager();
	
	void set_players_object(array <Npc*, MAX_USER + MAX_NPC + MAX_AI>& pls);

	void player_put(Npc* p);
	void player_move(Npc* p);
	void player_remove(Npc* p, bool dead);	// 사망이면 ture, Disconnect면 false
	void player_remove(Npc* p, bool dead, Npc* attacker);	// 사망이면 ture, Disconnect면 false
	int check_sector_id(int p_x, int p_z);

	// 해당되는 섹터 찾기
	void check_sector_put(Npc* p, int sector_id);
	void check_sector_move(Npc* p, int sector_id);
	void check_sector_remove(Npc* p, int sector_id, bool dead, Npc* attacker = nullptr);

	// viewlist 설정
	void check_viewlist_put(Npc* p, int sector_id);
	void check_viewlist_move(Npc* p, int sector_id);
	void check_viewlist_disconnect(Npc* p, int sector_id);
	void check_viewlist_dead(Npc* p, int sector_id);
};

