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
    bool skill_check;

protected:
    char	_name[MAX_NAME_SIZE];
    int     party_id;	//����(��Ƽ)������ ��Ƽ�� id
public:
    bool running_pattern;  // -1,0,1  ���������� ���� -1 ���¿��� �̵�, ���� �Ұ���, �ִϸ��̼� ��Ŷ ������ 0 ���� �ٲ㼭 ����// 
    bool    start_game;    //�ִϸ��̼� �϶�� ������(���� 1 ) ���ݰ� �̵��� �������// ��Ÿ�� ������ -1�� �ٲٰ�  
    bool    join_dungeon_room;
    bool running_attack;
    Partner(int id);

    ~Partner();

    void partner_move(Partner* pa, Gaia* gaia);
    void partner_attack(Partner* pa, Gaia* gaia);
    void partner_normal_attack(Partner* pa, Gaia* gaia);
    void attack_success(Partner* pa, Gaia* gaia, float atk_factor);

    bool check_inside(pos a, pos b, pos c, pos n);
    bool isInsideTriangle(pos a, pos b, pos c, pos n);

    int get_party_id();
    void set_party_id(int id);
};
