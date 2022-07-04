#pragma once
#include "stdafx.h"
class SkillBuf
{
private:
	int _id;		// �÷��̾� id
	int _type;	// 0 : ���ݷ��� �÷���
	float _percent;	// ������ �ۼ�Ʈ
	chrono::system_clock::time_point _timer;	// ��ų �ð���
public:
	SkillBuf(int id, int type, float percent,
		chrono::system_clock::time_point timer);
	~SkillBuf();

	constexpr bool operator < (const SkillBuf& _left) const
	{
		return (_timer > _left._timer);
	}
};

