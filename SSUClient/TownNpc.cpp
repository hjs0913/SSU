#include "stdafx.h"
#include "TownNpc.h"

const float TownNpc::m_x = 3231.f;
const float TownNpc::m_z = 483.f;
const float TownNpc::r = 235.f;
const float TownNpc::round90 = 90*3.141592/180;
float TownNpc::m_fTimeElapsed = 0.0f;

void TownNpc::UpdateTime(float fTimeElapsed)
{
	m_fTimeElapsed += (fTimeElapsed/2) * 0.0007f;
	if (m_fTimeElapsed >= round90 * 4) m_fTimeElapsed = 0.f;
}

XMFLOAT3 TownNpc::UpdatePosition(int i)
{
	float x = 0.f;
	float z = 0.f;
	switch (i) {
	case 0: {
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
		x = m_x + 171 * sin(m_fTimeElapsed+1);
		z = m_z;
		break;
	}
	case 5: {
		x = m_x;
		z = m_z + 171 * sin(m_fTimeElapsed);
		break;
	}
	case 6: {
		if (sin(m_fTimeElapsed) >= 0) {
			x = m_x + r * cos(m_fTimeElapsed/2);
			z = m_z + r * sin(m_fTimeElapsed/2);
		}
		else {
			if (m_fTimeElapsed < round90 * 3) {
				x = m_x;
				z = m_z + r*(1.f +  sin(m_fTimeElapsed));
			}
			else {
				x = m_x + r * cos(m_fTimeElapsed);
				z = m_z;
			}
		}
		break;
	}
	case 7: {
		if (sin(m_fTimeElapsed) >= 0) {
			x = m_x + r * cos(m_fTimeElapsed / 2 + round90);
			z = m_z + r * sin(m_fTimeElapsed / 2 + round90);
		}
		else {
			if (m_fTimeElapsed < round90 * 3) {
				x = m_x + r * cos(m_fTimeElapsed);
				z = m_z;
			}
			else {
				x = m_x;
				z = m_z + r * cos(m_fTimeElapsed);
			}
		}
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
	float x = 1.f;
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
		if (m_fTimeElapsed+1 > round90 && m_fTimeElapsed+1 < 3 * round90) x = -1.f;
		break;
	}
	case 5: {
		x = 0.f;
		if (m_fTimeElapsed > round90 && m_fTimeElapsed < 3 * round90) z = -1.f;
		else z = 1.f;
		break;
	}
	case 6: {
		if (sin(m_fTimeElapsed) >= 0) {
			x = cos((m_fTimeElapsed/2) + round90);
			z = sin((m_fTimeElapsed / 2) + round90);
		}
		else {
			if (m_fTimeElapsed < round90 * 3) {
				x = 0.f;
				z = -1.f;
			}
			else {
				x = 1.f;
				z = 0.f;
			}
		}
		break;
	}
	case 7: {
		if (sin(m_fTimeElapsed) >= 0) {
			x = -cos(m_fTimeElapsed / 2);
			z = -sin(m_fTimeElapsed / 2);
		}
		else {
			if (m_fTimeElapsed > round90 * 3) {
				x = 0.f;
				z = 1.f;
			}
		}
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