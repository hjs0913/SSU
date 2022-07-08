#include "Magician.h"

void Magician::Initialize(Player* pl)
{
    int lv = pl->get_lv();
    pl->set_maxhp(16 * lv * lv + 70 * lv);
    pl->set_hp(pl->get_maxhp());
    pl->set_maxmp(17 * lv * lv + 60 * lv);
    pl->set_mp(pl->get_maxmp());
    pl->set_physical_attack(0.1 * lv * lv + 5 * lv);
    pl->set_magical_attack(0.3 * lv * lv + 10 * lv);
    pl->set_physical_defence(0.17 * lv * lv + 10 * lv);
    pl->set_magical_defence(0.24 * lv * lv + 10 * lv);
    pl->set_basic_attack_factor(50.0f);
    pl->set_defence_factor(0.0002);
}

void Magician::first_skill(Player* pl, Npc* target)
{

}

void Magician::second_skill(Player* pl, Npc* target)
{

}

void Magician::third_skill(Player* pl)
{

}
