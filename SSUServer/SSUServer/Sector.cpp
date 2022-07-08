#include "Sector.h"
#include <algorithm>
Sector::Sector(int id)
{
	sector_id = id;
}

void Sector::add_player(int player_id)
{
	sector_lock.lock();
	players_id.push_back(player_id);
	sector_lock.unlock();
}

void Sector::erase_player(int player_id)
{
	sector_lock.lock();
	players_id.erase(std::find(players_id.begin(), players_id.end(), player_id));
	sector_lock.unlock();
}

std::vector<int> Sector::get_players()
{
	return players_id;
}
