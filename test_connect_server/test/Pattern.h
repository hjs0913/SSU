#pragma once

class CPattern
{
private:
public:
	bool pattern_on[7] = { false };

	XMFLOAT3 pattern_one[4];
	XMFLOAT3 pattern_two[3];
	int pattern_two_number;
	XMFLOAT3 pattern_two_look;
	XMFLOAT3 pattern_five;
	XMFLOAT3 pattern_five_look;

	CPattern();
	~CPattern();

	void set_pattern_one(int* x, int* z);
	void set_pattern_two(int* x, int* z);
};

