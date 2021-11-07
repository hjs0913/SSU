#pragma once
#include "protocol.h"

class BUFF {
public:
	BUF_TYPE _type;
	float _effect;
	float _time;
	bool _use;
public:
	BUFF();
	~BUFF();

	void buf_setting(BUF_TYPE type, float effect, float time);
};

class NUFF {
public:
	BUF_TYPE _type;
	float _effect;
	float _time;
	bool _use;
public:
	NUFF();
	~NUFF();

	void buf_setting(BUF_TYPE type, float effect, float time);
};

