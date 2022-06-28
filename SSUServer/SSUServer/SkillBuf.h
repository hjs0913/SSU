#pragma once
#include "stdafx.h"
class SkillBuf
{
private:
	int _id;		// 플레이어 id
	int _type;	// 0 : 공격력을 올려줌
	float _percent;	// 버프의 퍼센트
	chrono::system_clock::time_point _timer;	// 스킬 시간초
public:
	SkillBuf(int id, int type, float percent,
		chrono::system_clock::time_point timer);
	~SkillBuf();

	constexpr bool operator < (const SkillBuf& _left) const
	{
		return (_timer > _left._timer);
	}
};

