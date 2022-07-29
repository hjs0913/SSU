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
    int     target_id;  //가이아 보스의 아이디로 하자

    pos nearest = { 0,0, };
    float dis = 0.0;
    int nearest_num = 0;
    pos move;
    bool move_once;
    bool skill_check;

protected:
    char	_name[MAX_NAME_SIZE];
    int     party_id;	//서버(파티)에서의 파티원 id
public:
    bool running_pattern;  // -1,0,1  데미지들어가기 전에 -1 상태에선 이동, 공격 불가능, 애니메이션 패킷 보내면 0 으로 바꿔서 가능// 
    bool    start_game;    //애니메이션 하라고 보내면(상태 1 ) 공격과 이동을 멈춰야해// 쿨타임 지나면 -1로 바꾸고  
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
