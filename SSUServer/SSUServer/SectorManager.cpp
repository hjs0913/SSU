#include "SectorManager.h"
#include "send.h"

SectorManager::SectorManager()
{
	for (int i = 0; i < MAX_SECTOR; i++) {
		sectors[i] = new Sector(i);
		obs_sectors[i] = new Sector(i);
	}
}

void SectorManager::set_players_object(array <Npc*, MAX_USER + MAX_NPC>& pls)
{
	players = pls;
}

void SectorManager::player_put(Npc* p)
{
	int sector_id = check_sector_id(p->get_x(), p->get_z());
	sectors[sector_id]->add_player(p->get_id());
	
	p->vl.lock();
	p->set_sector_id(sector_id);
	p->vl.unlock();

	// 새로 들어온 섹터와 관련이 있는 플레이어에게 put send를 한다

	// 새로 들어온 사람에게 put 해준다
	switch (sector_id) {
	case 0: {	// 0, 1, 8, 9
		break;
	}
	case 7: {	// 6, 7, 14, 15
		break;
	}
	case 56: {	// 48, 49, 56, 57
		break;
	}
	case 63: {	// 54, 55, 62, 63
		break;
	}
	default: {
		if (sector_id / 8 == 0) {

		}
		else if (sector_id / 8 == 7) {

		}
		else if (sector_id % 8 == 0) {

		}
		else if (sector_id % 8 == 7) {

		}
		else {

		}
	}
	}
	send_put_packet_sector_player(p, sector_id);

	// 기존의 사람에게 새로 들어온 사람을 put해준다
}

void SectorManager::player_move(Npc* p)
{
	int new_sector_id = check_sector_id(p->get_x(), p->get_z());
	p->vl.lock();
	int origin_sector_id = p->get_sector_id();
	if (origin_sector_id != new_sector_id) {
		sectors[new_sector_id]->add_player(p->get_id());
		// 원래있던 섹터에서 플레이어 id를 뺀다
		sectors[origin_sector_id]->erase_player(p->get_id());
		p->set_sector_id(new_sector_id);
		p->vl.unlock();
		// 본래 섹터와 이동된 섹터 관련이 있는 플레이어에게 put, delete send를 한다
		send_put_packet_sector_player(p, new_sector_id);
		send_delete_packet_sector_player(p, origin_sector_id);
	}
	else{
		p->vl.unlock();
		// 이동 패킷을 해당되는 섹터에게 보내주자
	}
}

void SectorManager::player_erase(Npc* p)
{
	p->vl.lock();
	int origin_sector_id = p->get_sector_id();
	sectors[origin_sector_id]->erase_player(p->get_id());
	p->set_sector_id(-1);
	p->vl.unlock();

	// 본래 섹터와 관련이 있는 플레이어에게 delete send를 한다.
	send_delete_packet_sector_player(p, origin_sector_id);
}

int SectorManager::check_sector_id(int p_x, int p_z)
{
	int t_x = p_x / SECTOR_WIDTH;
	int t_z = p_z / SECTOR_HEIGHT;
	int sector_id = t_x + 8 * t_z;
	return sector_id;
}

void SectorManager::send_put_packet_sector_player(Npc* p, int sector_id)
{





	// 장애물 처리

	// 자기 자신
	if (p->get_tribe() == HUMAN) {
		Player* pl = reinterpret_cast<Player*>(p);
		for (int i : sectors[sector_id]->get_players()) {
			send_put_object_packet(pl, players[i]);
		}
		// 상하좌우 대각선

	}

	// 다른 사람에게

}

void SectorManager::send_delete_packet_sector_player(Npc* p, int sector_id)
{
	// 장애물 처리
}
