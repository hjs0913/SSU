#include "SectorManager.h"
#include "send.h"
#include "ObjectManager.h"

SectorManager::SectorManager()
{
	for (int i = 0; i < MAX_SECTOR; i++) {
		sectors[i] = new Sector(i);
		obs_sectors[i] = new Sector(i);
	}
}

void SectorManager::set_players_object(array <Npc*, MAX_USER + MAX_NPC + MAX_AI>& pls)
{
	players = pls;
}

void SectorManager::player_put(Npc* p)
{
	p->vl.lock();
	p->viewlist.clear();
	p->vl.unlock();
	int sector_id = check_sector_id(p->get_x(), p->get_z());
	sectors[sector_id]->add_player(p->get_id());
	
	p->sector_lock.lock();
	p->set_sector_id(sector_id);
	p->sector_lock.unlock();

	check_sector_put(p, sector_id);
}

void SectorManager::player_move(Npc* p)
{
	int new_sector_id = check_sector_id(p->get_x(), p->get_z());
	p->sector_lock.lock();
	int origin_sector_id = p->get_sector_id();
	if (origin_sector_id != new_sector_id && origin_sector_id != -1) {
		sectors[new_sector_id]->add_player(p->get_id());
		// 원래있던 섹터에서 플레이어 id를 뺀다
		sectors[origin_sector_id]->erase_player(p->get_id());
		p->set_sector_id(new_sector_id);
	}
	p->sector_lock.unlock();
	check_sector_move(p, new_sector_id);
	if(p->get_tribe() == HUMAN)
		send_move_packet(reinterpret_cast<Player*>(p), p, 1);
}

void SectorManager::player_remove(Npc* p, bool dead)
{
	p->sector_lock.lock();
	int origin_sector_id = p->get_sector_id();
	if (origin_sector_id != -1) {
		sectors[origin_sector_id]->erase_player(p->get_id());
		p->set_sector_id(-1);
	}
	p->sector_lock.unlock();

	check_sector_remove(p, origin_sector_id, dead);
}

void SectorManager::player_remove(Npc* p, bool dead, Npc* attacker)
{
	p->sector_lock.lock();
	int origin_sector_id = p->get_sector_id();
	if (origin_sector_id != -1) {
		sectors[origin_sector_id]->erase_player(p->get_id());
		p->set_sector_id(-1);
	}
	p->sector_lock.unlock();

	// 본래 섹터와 관련이 있는 플레이어에게 delete send를 한다.
	check_sector_remove(p, origin_sector_id, dead, attacker);
}


int SectorManager::check_sector_id(int p_x, int p_z)
{
	int t_x = p_x / SECTOR_WIDTH;
	int t_z = p_z / SECTOR_HEIGHT;
	int sector_id = t_x + 8 * t_z;
	return sector_id;
}

// 해당되는 섹터 찾기
void SectorManager::check_sector_put(Npc* p, int sector_id)
{
	// 새로 들어온 섹터와 관련이 있는 플레이어에게 put send를 한다
	int sector_id_x = sector_id % 8; // 나머지 관련
	int sector_id_z = sector_id / 8; // 나누기 몫 관련

	// 새로 들어온 사람에게 put 해준다
	switch (sector_id) {
	case 0: {	// 0, 1, 8, 9
		check_viewlist_put(p, 0);
		check_viewlist_put(p, 1);
		check_viewlist_put(p, 8);
		check_viewlist_put(p, 9);
		break;
	}
	case 7: {	// 6, 7, 14, 15
		check_viewlist_put(p, 6);
		check_viewlist_put(p, 7);
		check_viewlist_put(p, 14);
		check_viewlist_put(p, 15);
		break;
	}
	case 56: {	// 48, 49, 56, 57
		check_viewlist_put(p, 48);
		check_viewlist_put(p, 49);
		check_viewlist_put(p, 56);
		check_viewlist_put(p, 57);
		break;
	}
	case 63: {	// 54, 55, 62, 63
		check_viewlist_put(p, 54);
		check_viewlist_put(p, 55);
		check_viewlist_put(p, 62);
		check_viewlist_put(p, 63);
		break;
	}
	default: {
		if (sector_id / 8 == 0) {
			for (int i = -1; i <= 0; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_put(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id / 8 == 7) {
			for (int i = 0; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_put(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id % 8 == 0) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 0; j++) {
					check_viewlist_put(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id % 8 == 7) {
			for (int i = -1; i <= 1; i++) {
				for (int j = 0; j <= 1; j++) {
					check_viewlist_put(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_put(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
	}
	}
}

void SectorManager::check_sector_move(Npc* p, int sector_id)
{
	// 새로 들어온 섹터와 관련이 있는 플레이어에게 put send를 한다
	int sector_id_x = sector_id % 8; // 나머지 관련
	int sector_id_z = sector_id / 8; // 나누기 몫 관련

	// 새로 들어온 사람에게 put 해준다
	switch (sector_id) {
	case 0: {	// 0, 1, 8, 9
		check_viewlist_move(p, 0);
		check_viewlist_move(p, 1);
		check_viewlist_move(p, 8);
		check_viewlist_move(p, 9);
		break;
	}
	case 7: {	// 6, 7, 14, 15
		check_viewlist_move(p, 6);
		check_viewlist_move(p, 7);
		check_viewlist_move(p, 14);
		check_viewlist_move(p, 15);
		break;
	}
	case 56: {	// 48, 49, 56, 57
		check_viewlist_move(p, 48);
		check_viewlist_move(p, 49);
		check_viewlist_move(p, 56);
		check_viewlist_move(p, 57);
		break;
	}
	case 63: {	// 54, 55, 62, 63
		check_viewlist_move(p, 54);
		check_viewlist_move(p, 55);
		check_viewlist_move(p, 62);
		check_viewlist_move(p, 63);
		break;
	}
	default: {
		if (sector_id / 8 == 0) {
			for (int i = -1; i <= 0; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_move(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id / 8 == 7) {
			for (int i = 0; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_move(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id % 8 == 0) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 0; j++) {
					check_viewlist_move(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else if (sector_id % 8 == 7) {
			for (int i = -1; i <= 1; i++) {
				for (int j = 0; j <= 1; j++) {
					check_viewlist_move(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
		else {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					check_viewlist_move(p, (sector_id_z - i) * 8 + (sector_id_x - j));
				}
			}
		}
	}
	}
}

void SectorManager::check_sector_remove(Npc* p, int sector_id, bool dead, Npc* attacker)
{
	p->vl.lock();
	unordered_set<int> remove_vl = p->viewlist;
	p->vl.unlock();

	if (dead) {
		for (int i : remove_vl) {
			if (!static_ObjectManager::get_objManger()->is_npc(i))
				send_dead_packet(reinterpret_cast<Player*>(players[i]), attacker, p);
			players[i]->vl.lock();
			players[i]->viewlist.erase(p->get_id());
			players[i]->vl.unlock();
		}

		if(!static_ObjectManager::get_objManger()->is_npc(p->get_id()))
			send_dead_packet(reinterpret_cast<Player*>(p), attacker, p);
	}
	else {
		for (int i : remove_vl) {
			if (!static_ObjectManager::get_objManger()->is_npc(i))
				send_remove_object_packet(reinterpret_cast<Player*>(players[i]), p);
			players[i]->vl.lock();
			players[i]->viewlist.erase(p->get_id());
			players[i]->vl.unlock();
		}
	}
}

// viewlist 설정
void SectorManager::check_viewlist_put(Npc* p, int sector_id)
{
	sectors[sector_id]->sector_lock.lock();
	vector<int> sector_player = sectors[sector_id]->get_players();
	sectors[sector_id]->sector_lock.unlock();

	for (int i : sector_player) {
		if (i == p->get_id()) continue;
		players[i]->state_lock.lock();
		if (ST_INGAME != players[i]->get_state()) {
			players[i]->state_lock.unlock();
			continue;
		}
		players[i]->state_lock.unlock();
		if (false == static_ObjectManager::get_objManger()->is_near(i, p->get_id())) continue;

		p->vl.lock();
		p->viewlist.insert(i);
		p->vl.unlock();

		players[i]->vl.lock();
		players[i]->viewlist.insert(p->get_id());
		players[i]->vl.unlock();

		if (p->get_tribe() == HUMAN && players[i]->get_tribe() != HUMAN) {
			if (players[i]->get_move_active() == false) {
				players[i]->push_npc_move_event();
				players[i]->set_move_active(true);
			}
		}

		if (!static_ObjectManager::get_objManger()->is_npc(p->get_id())) 
			send_put_object_packet(reinterpret_cast<Player*>(p), players[i]);
		if (!static_ObjectManager::get_objManger()->is_npc(i)) 
			send_put_object_packet(reinterpret_cast<Player*>(players[i]), p);
	}
}

void SectorManager::check_viewlist_move(Npc* p, int sector_id)
{
	sectors[sector_id]->sector_lock.lock();
	vector<int> sector_player = sectors[sector_id]->get_players();
	sectors[sector_id]->sector_lock.unlock();

	for (int i : sector_player) {
		if (i == p->get_id()) continue;
		players[i]->state_lock.lock();
		if (ST_INGAME != players[i]->get_state()) {
			players[i]->state_lock.unlock();
			continue;
		}
		players[i]->state_lock.unlock();
		if (false == static_ObjectManager::get_objManger()->is_near(i, p->get_id())) {
			// 시야에서 사라진건지 확인
			p->vl.lock();
			if (p->viewlist.count(i)) {	// 시야에 있다가 사라진 플레이어
				p->viewlist.erase(i);
				p->vl.unlock();

				players[i]->vl.lock();
				players[i]->viewlist.erase(p->get_id());
				players[i]->vl.unlock();

				if (!static_ObjectManager::get_objManger()->is_npc(p->get_id()))
					send_remove_object_packet(reinterpret_cast<Player*>(p), players[i]);
				if (!static_ObjectManager::get_objManger()->is_npc(i))
					send_remove_object_packet(reinterpret_cast<Player*>(players[i]), p);

			}
			else p->vl.unlock();
		}
		else {
			p->vl.lock();
			if ( 0 != p->viewlist.count(i)) {
				// 계속 시야에 존재하는 플레이어
				p->vl.unlock();
				if (!static_ObjectManager::get_objManger()->is_npc(p->get_id())) {
					send_move_packet(reinterpret_cast<Player*>(p), players[i], 1);
					//send_look_packet(reinterpret_cast<Player*>(p), players[i]);
				}
			}
			else {
				// 새로 시야에 생긴 플레이어
				p->viewlist.insert(i);
				p->vl.unlock();

				players[i]->vl.lock();
				players[i]->viewlist.insert(p->get_id());
				players[i]->vl.unlock();

				if (p->get_tribe()==HUMAN && players[i]->get_tribe() != HUMAN) {
					if (players[i]->get_move_active() == false) {
						players[i]->push_npc_move_event();
						players[i]->set_move_active(true);
					}
				}

				if (!static_ObjectManager::get_objManger()->is_npc(p->get_id()))
					send_put_object_packet(reinterpret_cast<Player*>(p), players[i]);
				if (!static_ObjectManager::get_objManger()->is_npc(i))
					send_put_object_packet(reinterpret_cast<Player*>(players[i]), p);
			}
		}
	}
}
