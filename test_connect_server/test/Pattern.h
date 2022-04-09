#pragma once

class CPattern
{
private:
public:
	bool pattern_on[7] = { false };
	XMFLOAT3 pattern_one[4];

	CPattern();
	~CPattern();

	void set_pattern_one(int* x, int* z);

};

