#include "CBUFF.h"

BUFF::BUFF()
{
	_use = false;
	_type = B_NONE;
	_effect = 0.0f;
	_time = 0.0f;
}

BUFF::~BUFF() {};

void BUFF::buf_setting(BUF_TYPE type, float effect, float time)
{
	_type = type;
	_effect = effect;
	_time = time;
}

NUFF::NUFF()
{
	_use = false;
	_type = B_NONE;
	_effect = 0.0f;
	_time = 0.0f;
}

NUFF::~NUFF() {};

void NUFF::buf_setting(BUF_TYPE type, float effect, float time)
{
	_type = type;
	_effect = effect;
	_time = time;
}
