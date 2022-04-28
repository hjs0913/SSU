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
    int     target_id;  //가이아 보스의 아이디로 하자

protected:
    char	_name[MAX_NAME_SIZE];

    bool    running_pattern;

    int     party_id;	//서버(파티)에서의 파티원 id
public:
    bool    start_game;
    bool    join_dungeon_room;

    Partner(int id);

    ~Partner();

    void partner_move();
    void partner_attack();

    void physical_skill_success(int p_id, int target, float skill_factor);

    bool check_inside(Coord_P a, Coord_P b, Coord_P c, Coord_P n) {
        Coord_P A, B, C;
        A.x = b.x - a.x;
        A.z = b.z - a.z;
        B.x = c.x - a.x;
        B.z = c.z - a.z;
        C.x = n.x - a.x;
        C.z = n.z - a.z;

        if ((A.x * B.z - A.z * B.x) * (A.x * C.z - A.z * C.x) < 0)
            return false;
        return true;
    }
    bool isInsideTriangle(Coord_P a, Coord_P b, Coord_P c, Coord_P n)
    {
        if (!check_inside(a, b, c, n)) return false;
        if (!check_inside(b, c, a, n)) return false;
        if (!check_inside(c, a, b, n)) return false;
        return true;
    }
};
