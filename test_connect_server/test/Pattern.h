#pragma once

class CPattern
{
private:
	XMFLOAT3 pattern_one[4];
public:
	bool pattern_on[7] = { false };

	CPattern();
	~CPattern();

	void set_pattern_one(int* x, int* z);

};

