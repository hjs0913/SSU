#pragma once
#include "stdafx.h"
#include "Player.h"

namespace Supporter
{
	void Initialize(Player* pl);
	void first_skill(Player* pl, Npc* target);	// 버프
	void second_skill(Player* pl, Npc* target);	// 버프
	void third_skill(Player* pl);	// 버프
};

