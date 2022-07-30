#include "stdafx.h"
#include "SoundManager.h"

SoundManager* SoundManager::m_SoundManager = nullptr;

SoundManager::SoundManager()
{
	m_index = 0;
	m_SoundList.clear();
	m_pSystem = nullptr;
}

bool SoundManager::Init()
{
	FMOD_RESULT ret;
	ret = FMOD::System_Create(&m_pSystem);
	if (ret != FMOD_OK) {
		cout << "FMOD Create ����" << endl;
		return false;
	}
	
	// FMOD �ý��� �ʱ�ȣ ä�� 32æ���� ���� ���� �ý��� �ʱ�ȭ
	// ������ų(4*3) + ������ų(3) + ���� ����(6) + ����� 2�� = 12 + 3 + 6 + 2 = 23
	
	ret = m_pSystem->init(32, FMOD_INIT_NORMAL, 0);
	if (ret != FMOD_OK) {
		cout << "FMOD Init ����" << endl;
		return false;
	}
	return true;
}

void SoundManager::SettingSound()
{
	m_SoundManager->LoadSound(L"Sound/Login.mp3");	//0
	m_SoundManager->LoadSound(L"Sound/OpenWorld.mp3");	//1
	m_SoundManager->LoadSound(L"Sound/indun.mp3");	//2

	// Monster attack
	m_SoundManager->LoadSound(L"Sound/Rabbit_attack.wav");	//3
	m_SoundManager->LoadSound(L"Sound/Spider_attack.wav");	//4
	m_SoundManager->LoadSound(L"Sound/Frog_attack.wav");	//5
	m_SoundManager->LoadSound(L"Sound/Monkey_attack.wav");	//6
	m_SoundManager->LoadSound(L"Sound/Wolf_attack.wav");	//7
	m_SoundManager->LoadSound(L"Sound/Pig_attack.wav");		//8

	// Boss Skill
	m_SoundManager->LoadSound(L"Sound/Gaia_chamkyuck.wav");	//9
	m_SoundManager->LoadSound(L"Sound/Gaia_jangpan.wav");	//10
	m_SoundManager->LoadSound(L"Sound/Gaia_wave.wav");		//11
}

void SoundManager::Relase()
{
	for (auto s : m_SoundList) {
		s.m_sound->Release();
		delete s.m_sound;
	}
	m_SoundList.clear();
	m_pSystem->close();
	m_pSystem->release();
}

void SoundManager::FrameAdvance()
{
	m_pSystem->update();
}

CSound* SoundManager::LoadSound(wstring filename)
{
	FMOD_RESULT ret;

	TCHAR szFileName[MAX_PATH] = { 0, };
	TCHAR Drive[MAX_PATH] = { 0, };
	TCHAR Dir[MAX_PATH] = { 0, };
	TCHAR FileName[MAX_PATH] = { 0, };
	TCHAR FileExt[MAX_PATH] = { 0, };

	wstring fullpathname = filename;
	_tsplitpath_s(fullpathname.c_str(), Drive, Dir, FileName, FileExt);

	// �ߺ� ����
	for (auto s : m_SoundList) {
		if (s.m_sound->m_name == FileName) return s.m_sound;
	}

	// ���� ���� �޸� �ε� ������
	CSound* sound = new CSound(m_pSystem, m_index, FileName);

	// ���� ����
	std::string mutiname;	// ���⼭ Wstirng -> string���� �ٲٱ�
	mutiname.assign(fullpathname.begin(), fullpathname.end());

	ret = m_pSystem->createSound(mutiname.c_str(), FMOD_DEFAULT, 0, &sound->m_pSound);
	if (ret != FMOD_OK) {
		sound->Release();
		return nullptr;
	}
	m_SoundList.push_back(Soundlist(m_index, sound));
	m_index++;
	return sound;
}

CSound* SoundManager::GetSound(int index)
{
	auto iter = find_if(m_SoundList.begin(), m_SoundList.end(), [index](Soundlist s) {return s.index == index; });
	if (iter != m_SoundList.end()) return (*iter).m_sound;
}
