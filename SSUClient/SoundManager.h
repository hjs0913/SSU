#pragma once
#include "CSound.h"
#include <list>

struct Soundlist {
	int index;
	CSound* m_sound;

	Soundlist() : index(0), m_sound(nullptr) {};
	Soundlist(int i, CSound* s) : index(i), m_sound(s) {};
};

class SoundManager
{
private:
	static SoundManager* m_SoundManager;

	int m_index;
	FMOD::System* m_pSystem = nullptr;
	vector<Soundlist> m_SoundList;


public:
	static SoundManager* GetSoundManager()
	{
		if (!m_SoundManager) m_SoundManager = new SoundManager;
		return m_SoundManager;
	}

	SoundManager();
	~SoundManager() {};

	bool Init();
	void Relase();
	void FrameAdvance();

	void SettingSound();

	CSound* LoadSound(wstring filename);

	CSound* GetSound(int index);
};

