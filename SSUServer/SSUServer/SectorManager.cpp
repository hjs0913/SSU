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

	// ���� ���� ���Ϳ� ������ �ִ� �÷��̾�� put send�� �Ѵ�

	// ���� ���� ������� put ���ش�
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

	// ������ ������� ���� ���� ����� put���ش�
}

void SectorManager::player_move(Npc* p)
{
	int new_sector_id = check_sector_id(p->get_x(), p->get_z());
	p->vl.lock();
	int origin_sector_id = p->get_sector_id();
	if (origin_sector_id != new_sector_id) {
		sectors[new_sector_id]->add_player(p->get_id());
		// �����ִ� ���Ϳ��� �÷��̾� id�� ����
		sectors[origin_sector_id]->erase_player(p->get_id());
		p->set_sector_id(new_sector_id);
		p->vl.unlock();
		// ���� ���Ϳ� �̵��� ���� ������ �ִ� �÷��̾�� put, delete send�� �Ѵ�
		send_put_packet_sector_player(p, new_sector_id);
		send_delete_packet_sector_player(p, origin_sector_id);
	}
	else{
		p->vl.unlock();
		// �̵� ��Ŷ�� �ش�Ǵ� ���Ϳ��� ��������
	}
}

void SectorManager::player_erase(Npc* p)
{
	p->vl.lock();
	int origin_sector_id = p->get_sector_id();
	sectors[origin_sector_id]->erase_player(p->get_id());
	p->set_sector_id(-1);
	p->vl.unlock();

	// ���� ���Ϳ� ������ �ִ� �÷��̾�� delete send�� �Ѵ�.
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





	// ��ֹ� ó��

	// �ڱ� �ڽ�
	if (p->get_tribe() == HUMAN) {
		Player* pl = reinterpret_cast<Player*>(p);
		for (int i : sectors[sector_id]->get_players()) {
			send_put_object_packet(pl, players[i]);
		}
		// �����¿� �밢��

	}

	// �ٸ� �������

}

void SectorManager::send_delete_packet_sector_player(Npc* p, int sector_id)
{
	// ��ֹ� ó��
}
