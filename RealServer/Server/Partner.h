#pragma once
#include "Player.h"
#include "SkillBuf.h"

class Partner : public Player
{
private:

protected:
    Player* party[GAIA_ROOM];	// 파티원 정보
    int dungeon_id;
    int player_cnt;
    DUNGEON_STATE st;
    int target_id;   //가이아 보스의 아이디로 하자
    bool running_pattern;

    int* party_id;	//서버에서의 파티원 id
    int                 _login_id;
    int		            _exp;
    JOB                 _job;
    atomic_bool	        _attack_active;	
    atomic_bool         _skill_active[3] = { false };
	int  partner_death_count = 3;

public:
    Player* partner;
    mutex state_lock;
    int player_rander_ok;
    bool start_game;

    Partner(int id);

    ~Partner()
    {
        closesocket(_socket);
    }
    int get_dungeon_id();
    void join_player(Player* pl);
    void partner_move();
    void partner_attack();
};