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

protected:
    char	_name[MAX_NAME_SIZE];

    Partner* partner_party[GAIA_ROOM];	// 파트너 정보
    int* partner_party_id;	//서버에서의 파티원 id

    int dungeon_id;


    DUNGEON_STATE st;
    int target_id;   //가이아 보스의 아이디로 하자
    bool running_pattern;

    int* party_id;	//서버에서의 파티원 id
    int level;
    int                 _login_id;
    int		            _exp;
    JOB                 _job;
    atomic_bool	        _attack_active;
    atomic_bool         _skill_active[3] = { false };
    int  partner_death_count = 3;

public:

    int player_cnt;
    Player* partner;
    mutex state_lock;
    int player_rander_ok;
    bool start_game;
    bool join_dungeon_room;

    Partner(int id);

    ~Partner();

    int get_dungeon_id();
    void join_player(Player* pl);
    void partner_move();
    void partner_attack();


    void set_level(int lv);

    char* partner_get_name();
    int partner_get_id();
    int partner_get_lv();
    JOB partner_get_job();

    void join_partner(Partner* pa);
    Partner** get_party_partner();
    DUNGEON_STATE get_dun_st();
    void set_dun_st(DUNGEON_STATE dst);
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
extern array <Partner*, MAX_USER * 3> partners;
