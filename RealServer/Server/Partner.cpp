#include "send.h"
#include "Partner.h"
#include <random>
#include <ctime>


Partner::Partner(int d_id) : Player(d_id)
{
	start_game = false;
	target_id = 0;
	join_dungeon_room = false;

	running_pattern = false;
}

Partner::~Partner()
{
	
}

void Partner::partner_move()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	int p = pattern(gen);
	switch (p % 4) {
	case 0: {
		_x += 10;
		break;
	}
	case 1: {
		_x -= 10;
		break;
	}
	case 2: {
		_z += 10;
		break;
	}
	case 3: {
		_z -= 10;
		break;
	}

	}

}
void Partner::physical_skill_success(int p_id, int target, float skill_factor)
{

}

void Partner::partner_attack()  //�Ϲݰ��� �⺻, ��ų�� ��Ÿ�� �������� ��� ������ ���� 
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen);

}