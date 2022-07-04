#pragma once
#include "SkillBuf.h"
#include "Gaia.h"
#include "stdafx.h"

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




public:
    bool    start_game;
    bool    join_dungeon_room;
    int     party_id;	//����(��Ƽ)������ ��Ƽ�� id
    Partner(int id);

    ~Partner();

    void partner_move(Partner* pa, Gaia* gaia);
    void partner_attack(Partner* pa, Gaia* gaia);
    void partner_normal_attack(Partner* pa, Gaia* gaia);
    void attack_success(Partner* pa, Gaia* gaia, float atk_factor);

    bool check_inside(pos a, pos b, pos c, pos n);
    bool isInsideTriangle(pos a, pos b, pos c, pos n);
    int get_party_id();

};
