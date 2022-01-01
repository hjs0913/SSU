#include "SkillBuf.h"

SkillBuf::SkillBuf(int id, int type, float percent,
	chrono::system_clock::time_point timer)
{
	_id = id;
	_type = type;
	_percent = percent;
	_timer = timer;
}

SkillBuf::~SkillBuf() {}
