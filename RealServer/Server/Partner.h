#pragma once
#include "SkillBuf.h"
#include "Player.h"
#include "Gaia.h"

struct Coord_P
{
    float x;
    float z;
};

class Partner : public Player
{
private:
    int     target_id;  //���̾� ������ ���̵�� ����

    pos nearest = { 0,0, };
    float dis = 0.0;
    int nearest_num = 0;
    pos move;
    bool move_once;
    bool running_pattern;

protected:
    char	_name[MAX_NAME_SIZE];



    int     party_id;	//����(��Ƽ)������ ��Ƽ�� id
public:
    bool    start_game;
    bool    join_dungeon_room;

    Partner(int id);

    ~Partner();

    void partner_move(Partner* pa, Gaia* gaia);
    void partner_attack(Partner* pa, Gaia* gaia);

    void physical_skill_success(int p_id, int target, float skill_factor);

    bool check_inside(pos a, pos b, pos c, pos n);
    bool isInsideTriangle(pos a, pos b, pos c, pos n);


};
