#include "stdafx.h"
#include "CSound.h"

CSound::CSound(FMOD::System* system, int index, std::wstring name)
{
	m_pSystem = system;
	m_id = index;
	m_name = name;
	m_play = false;
}

void CSound::SoundPlay(bool bloop) 
{
	if (m_pChannel != nullptr) {
		m_pChannel->isPlaying(&m_play);
	}
	if (m_play == false) {
		FMOD_RESULT ret = m_pSystem->playSound(m_pSound, nullptr, false, &m_pChannel);
		if (ret == FMOD_OK) {
			if (bloop) m_pChannel->setMode(FMOD_LOOP_NORMAL);
			else m_pChannel->setMode(FMOD_LOOP_OFF);
		}
	}
}

void CSound::SoundStop()
{
	if (m_pChannel != nullptr) {
		m_pChannel->stop();
	}
}

void CSound::SoundPause()
{
	m_pChannel->isPlaying(&m_play);
	if (m_play) m_play = false;
	else m_play = true;
	m_pChannel->setPaused(m_play);
}

void CSound::Release()
{
	if (m_pSound) {
		m_pSound->release();
		m_pSound = nullptr;
	}
}