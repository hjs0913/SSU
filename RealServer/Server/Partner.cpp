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
	
}
void Partner::physical_skill_success(int p_id, int target, float skill_factor)
{

}

void Partner::partner_attack()  //일반공격 기본, 스킬을 쿨타임 돌때마다 계속 쓰도록 하자 
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> pattern(0, 99);
	timer_event ev;

	int p = pattern(gen);

}