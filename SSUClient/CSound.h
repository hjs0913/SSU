#pragma once
#include "Sound/inc/fmod.hpp"
#include "Sound/inc/fmod_errors.h"
#pragma comment(lib, "fmod_vc.lib")


class CSound
{
private:
public:
	FMOD::Channel* m_pChannel = nullptr;
	FMOD::System* m_pSystem = nullptr;
	FMOD::Sound* m_pSound = nullptr;
	

	wstring m_name;
	bool	m_play;
	int		m_id;


	CSound() {};
	CSound(FMOD::System* system, int index, std::wstring name);
	~CSound() {};

	void SoundPlay(bool bloop);	// »ç¿îµå Àç»ý
	void SoundStop();			// »ç¿îµå ²ô±â
	void SoundPause();			// »ç¿îµå ¸ØÃã

	void Release();

};

