#pragma once
#include "stdafx.h"
#include "Player.h"

namespace Supporter
{
	void Initialize(Player* pl);
	void first_skill(Player* pl, Npc* target);	// ����
	void second_skill(Player* pl, Npc* target);	// ����
	void third_skill(Player* pl);	// ����
};

