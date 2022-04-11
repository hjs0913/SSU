#include "stdafx.h"
#include "Pattern.h"
#include <iostream>
using namespace std;

CPattern::CPattern()
{
	for (int i = 0; i < 4; i++) pattern_one[i] = XMFLOAT3(0, 0, 0);
}

CPattern::~CPattern()
{

}

void CPattern::set_pattern_one(int* x, int* z)
{
	for (int i = 0; i < 4; i++) {
		pattern_one[i].x = x[i];
		pattern_one[i].z = z[i];
	}
}

void CPattern::set_pattern_two(int* x, int* z)
{
	for (int i = 0; i < 3; i++) {
		pattern_two[i].x = x[i];
		pattern_two[i].z = z[i];
	}
}
