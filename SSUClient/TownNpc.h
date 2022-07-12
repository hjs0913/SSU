#pragma once
class TownNpc
{
private:
public:
	const static float m_x;
	const static float m_z;
	const static float r;
	const static float round90;
	static float m_fTimeElapsed;

	TownNpc();
	~TownNpc();

	static void UpdateTime(float fTimeElapsed);
	static XMFLOAT3 UpdatePosition(int i);
	static XMFLOAT3 UpdateLook(int i);
};

