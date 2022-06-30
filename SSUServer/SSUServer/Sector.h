#pragma once
#include <vector>
class Sector
{
private:
	int sector_id;
	std::vector<int> players_id;

public:
	Sector(int id);
	void add_player(int player_id);
	void erase_player(int player_id);

	std::vector<int> get_players();
};

