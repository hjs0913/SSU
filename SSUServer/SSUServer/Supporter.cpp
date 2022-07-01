#include "Supporter.h"

void Supporter::Initialize(Player* pl)
{
    int lv = pl->get_lv();
    pl->set_maxhp(18 * lv * lv + 70 * lv);
    pl->set_hp(pl->get_maxhp());
    pl->set_maxmp(15 * lv * lv + 60 * lv);
    pl->set_mp(pl->get_maxmp());
    pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
    pl->set_magical_attack(0.25 * lv * lv + 8 * lv);
    pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
    pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
    pl->set_basic_attack_factor(50.0f);
    pl->set_defence_factor(0.0002);

    pl->set_origin_physical_attack(pl->get_physical_attack());
    pl->set_origin_magical_attack(pl->get_magical_attack());
    pl->set_origin_physical_defence(pl->get_physical_defence());
    pl->set_origin_magical_defence(pl->get_magical_defence());

}

void Supporter::first_skill(Player* pl, Npc* target)
{

}

void Supporter::second_skill(Player* pl, Npc* target)
{

}

void Supporter::third_skill(Player* pl)
{

}
