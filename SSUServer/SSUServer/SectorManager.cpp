#include "SectorManager.h"

SectorManager::SectorManager()
{
	for (int i = 0; i < MAX_SECTOR; i++) {
		sectors[i] = new Sector(i);
	}
}

void SectorManager::player_accept(Npc* p)
{
	int sector_id = check_sector_id(p->get_x(), p->get_z());
	sectors[sector_id]->add_player(p->get_id());
}

void SectorManager::player_move(Npc* p)
{
	int sector_id = check_sector_id(p->get_x(), p->get_z());
}

void SectorManager::player_disconnect(int player_id)
{

}

int SectorManager::check_sector_id(int p_x, int p_z)
{
	int t_x = p_x / SECTOR_WIDTH;
	int t_z = p_z / SECTOR_HEIGHT;
	int sector_id = t_x + 8 * t_z;
	return sector_id;
}