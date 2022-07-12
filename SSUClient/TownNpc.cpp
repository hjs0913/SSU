#include "stdafx.h"
#include "TownNpc.h"

const float TownNpc::m_x = 3231.f;
const float TownNpc::m_z = 483.f;
const float TownNpc::r = 235.f;
const float TownNpc::round90 = 90*3.141592/180;
float TownNpc::m_fTimeElapsed = 0.0f;

void TownNpc::UpdateTime(float fTimeElapsed)
{
	m_fTimeElapsed += fTimeElapsed * 0.0005f;
}

XMFLOAT3 TownNpc::UpdatePosition(int i)
{
	float x = 0.f;
	float z = 0.f;
	switch (i) {
	case 0: {
		cout << m_fTimeElapsed << endl;
		x = m_x + r * cos(m_fTimeElapsed);
		z = m_z + r * sin(m_fTimeElapsed);
		break;
	}
	case 1: {
		x = m_x + r * cos(m_fTimeElapsed + 200.f);
		z = m_z + r * sin(m_fTimeElapsed + 200.f);
		break;
	}
	case 2: {
		x = m_x + r * cos(-m_fTimeElapsed);
		z = m_z + r * sin(-m_fTimeElapsed);
		break;
	}
	case 3: {
		x = m_x + r * cos(-m_fTimeElapsed - 200.f);
		z = m_z + r * sin(-m_fTimeElapsed - 200.f);
		break;
	}
	case 4: {
		break;
	}
	case 5: {
		break;
	}
	case 6: {
		break;
	}
	case 7: {
		break;
	}
	case 8: {
		break;
	}
	case 9: {
		break;
	}
	}

	return XMFLOAT3(x, 0.f, z);
}

XMFLOAT3 TownNpc::UpdateLook(int i)
{
	float x = 0.f;
	float z = 0.f;
	switch (i) {
	case 0: {
		x = cos(m_fTimeElapsed + round90);
		z = sin(m_fTimeElapsed + round90);
		break;
	}
	case 1: {
		x = cos(m_fTimeElapsed + 200 + round90);
		z = sin(m_fTimeElapsed + 200 + round90);
		break;
	}
	case 2: {
		x = cos(-m_fTimeElapsed - round90);
		z = sin(-m_fTimeElapsed - round90);
		break;
	}
	case 3: {
		x = cos(-m_fTimeElapsed - 200.f - round90);
		z = sin(-m_fTimeElapsed - 200.f - round90);
		break;
	}
	case 4: {
		break;
	}
	case 5: {
		break;
	}
	case 6: {
		break;
	}
	case 7: {
		break;
	}
	case 8: {
		break;
	}
	case 9: {
		break;
	}
	}

	return XMFLOAT3(x, 0.f, z);
}