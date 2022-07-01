#include "Tanker.h"

void Tanker::Initialize(Player* pl)
{
    int lv = pl->get_lv();
    pl->set_maxhp(22 * lv * lv + 80 * lv);
    pl->set_hp(pl->get_maxhp());
    pl->set_maxmp(8.5 * lv * lv + 50 * lv);
    pl->set_mp(pl->get_maxmp());
    pl->set_physical_attack(0.25 * lv * lv + 10 * lv);
    pl->set_magical_attack(0.08 * lv * lv + 5 * lv);
    pl->set_physical_defence(0.27 * lv * lv + 10 * lv);
    pl->set_magical_defence(0.2 * lv * lv + 10 * lv);
    pl->set_basic_attack_factor(50.0f);
    pl->set_defence_factor(0.0002);

    pl->set_origin_physical_attack(pl->get_physical_attack());
    pl->set_origin_magical_attack(pl->get_magical_attack());
    pl->set_origin_physical_defence(pl->get_physical_defence());
    pl->set_origin_magical_defence(pl->get_magical_defence());

}

void Tanker::first_skill(Player* pl, Npc* target)
{

}

void Tanker::second_skill(Player* pl, Npc* target)
{

}

void Tanker::third_skill(Player* pl)
{

}
