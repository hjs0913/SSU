//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
//#include "../RealServer/Server/protocol.h"
#include "../SSUServer/SSUServer/protocol.h"
#include "Network.h"
#include "TownNpc.h"
ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

bool point_light_bool = false;

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 2;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	
	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = SPOT_LIGHT;
	m_pLights[0].m_fRange = 200.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_pLights[0].m_fFalloff = 38.0f;
	m_pLights[0].m_fPhi = (float)cos(XMConvertToRadians(50.0f));
	m_pLights[0].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	//m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	//m_pLights[1].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	//m_pLights[1].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 1.0f);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 200); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.1f, 0.1f, 0.1f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/HeightMap.raw"), 512, 512, xmf3Scale, xmf4Color);


	m_nHierarchicalGameObjects = 3 + MAX_NPC + NUM_PLAYER + NUM_TOWN_NPC +  609;	// 성벽 1 + 집 2 + 몬스터 MAX_NPC(180) + 플레이어 30 + 마을 내에 NPC 10명+ 나무609
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];

	//for (int i = 0; i < m_nHierarchicalGameObjects; ++i) m_ppHierarchicalGameObjects[i] = NULL;


	//CLoadedModelInfo* pCastleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Castle_Wall.bin", NULL);
	//m_ppHierarchicalGameObjects[0] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCastleModel, 0);
	//m_ppHierarchicalGameObjects[0]->SetPosition(3200.0f, m_pTerrain->GetHeight(3200.0f, 970.0f), 970.0f);
	//m_ppHierarchicalGameObjects[0]->SetScale(15.0f, 15.0f, 12.0f);

	CLoadedModelInfo* pCastleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/BuildingMap.bin", NULL);
	m_ppHierarchicalGameObjects[0] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCastleModel, 0);
	m_ppHierarchicalGameObjects[0]->SetPosition(3270.0f, m_pTerrain->GetHeight(3270.0f, 460.0f), 460.0f);
	m_ppHierarchicalGameObjects[0]->SetScale(10.0f, 10.0f, 10.0f);

	if (pCastleModel) delete pCastleModel;

	CLoadedModelInfo* pHouseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building_5.bin", NULL);
	m_ppHierarchicalGameObjects[1] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHouseModel, 0);
	m_ppHierarchicalGameObjects[1]->SetPosition(2800.0f, m_pTerrain->GetHeight(2800.0f, 400.0f), 400.0f);
	m_ppHierarchicalGameObjects[1]->SetScale(1.0f, 1.0f, 1.0f);
	m_ppHierarchicalGameObjects[1]->Rotate(0.0f, 180.0f, 0.0f);

	CLoadedModelInfo* pHouseModell = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building_1.bin", NULL);
	m_ppHierarchicalGameObjects[2] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHouseModell, 0);
	m_ppHierarchicalGameObjects[2]->SetPosition(2790.0f, m_pTerrain->GetHeight(2790.0f, 550.0f), 550.0f);
	m_ppHierarchicalGameObjects[2]->SetScale(1.0f, 1.0f, 1.0f);

	if (pHouseModel) delete pHouseModel;
	if (pHouseModell) delete pHouseModell;

	pWolfModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Beast_Wolf.bin", NULL);
	monster_anim_cnt = pWolfModel->m_pAnimationSets->m_nAnimationSets;
	pSpiderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Spidermon.bin", NULL);
	pMonkeyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monkeymon.bin", NULL);
	pRabbitModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Rabbitmon.bin", NULL);
	pFrogModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Frogmon.bin", NULL);
	pPigModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Pigmon.bin", NULL);

	for (int i = 3; i < 3+MAX_NPC; ++i) {
		int temp_id = i - 3;
		if (temp_id / NPC_INTERVAL == 0)
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRabbitModel, monster_anim_cnt);
		else if (temp_id / NPC_INTERVAL == 1)
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, monster_anim_cnt);	
		else if (temp_id / NPC_INTERVAL == 2)
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pFrogModel, monster_anim_cnt); 
		else if (temp_id / NPC_INTERVAL == 3)
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonkeyModel, monster_anim_cnt);
		else if (temp_id / NPC_INTERVAL == 4)
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWolfModel, monster_anim_cnt);
		else if (temp_id / NPC_INTERVAL == 5)	
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pPigModel, monster_anim_cnt);
			

		for (int j = 0; j < monster_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			//m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 0.05f;
		}
		// 2번 부터 ANIMATION_TYPE_ONCE
		for (int j = 3; j < monster_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
		}
		m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[i]->SetPosition(0.f, -100.f, 0.f);
		m_ppHierarchicalGameObjects[i]->SetPosition(0.f, -100.f, 0.f);
	

		if (temp_id / NPC_INTERVAL == 0)		// 토끼
			m_ppHierarchicalGameObjects[i]->SetScale(15.0f, 15.0f, 15.0f);
		else if (temp_id / NPC_INTERVAL == 1)	// 거미
			m_ppHierarchicalGameObjects[i]->SetScale(15.0f, 15.0f, 15.0f);
		else if (temp_id / NPC_INTERVAL == 2)	// 개구리
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
		else if (temp_id / NPC_INTERVAL == 3)	// 원숭이
			m_ppHierarchicalGameObjects[i]->SetScale(20.0f, 20.0f, 20.0f);
		else if (temp_id / NPC_INTERVAL == 4)	// 늑대
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
		else if (temp_id / NPC_INTERVAL == 5)	// 돼지
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
	}

	if (pWolfModel) delete pWolfModel;
	if (pMonkeyModel) delete pMonkeyModel;
	if (pRabbitModel) delete pRabbitModel;
	if (pFrogModel) delete pFrogModel;
	if (pSpiderModel) delete pSpiderModel;
	if (pPigModel) delete pPigModel;

	pBastardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bastard_Warrior.bin", NULL);
	player_anim_cnt = pBastardModel->m_pAnimationSets->m_nAnimationSets;

	pTankerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Mighty_Warrior.bin", NULL);
	pSupporterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Priestess.bin", NULL);
	pMagicianModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Wizard_Girl.bin", NULL);

	for (int i = 3+MAX_NPC; i < 3+MAX_NPC+NUM_PLAYER; ++i) {
		m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSupporterModel, player_anim_cnt);
		for (int j = 0; j < player_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			//m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 0.05f;
		}
		m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_ppHierarchicalGameObjects[i]->SetPosition(0.f, -100.f, 0.f);
		m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);

		// 2번 부터 ANIMATION_TYPE_ONCE
		for (int j = 2; j < player_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
		}
	}

	for (int i = 3 + MAX_NPC + NUM_PLAYER; i < 3 + MAX_NPC + NUM_PLAYER + NUM_TOWN_NPC; i++) {
		switch (i%4)
		{
		case 0 : m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBastardModel, player_anim_cnt);
			break;
		case 1: m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTankerModel, player_anim_cnt);
			break;
		case 2: m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSupporterModel, player_anim_cnt);
			break;
		case 3: m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicianModel, player_anim_cnt);
			break;
		default:
			break;
		}
		for (int j = 0; j < player_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
		}
		m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[i]->SetPosition(0.f, -100.f, 0.f);
		m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);

	}

	// Magicial Skill Model Load
	pMagicainSkillModel1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Asteroid_1.bin", NULL);
	pMagicainSkillModel2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Asteroid_2.bin", NULL);
	
	// Tree Model Load and setPosition
	pTreeModel1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree1.bin", NULL);
	pTreeModel2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree2.bin", NULL);
	pTreeModel3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree3.bin", NULL);

	SetTreePosition(pd3dDevice, pd3dCommandList, 3 + MAX_NPC + NUM_PLAYER + NUM_TOWN_NPC, m_nHierarchicalGameObjects-1);

	if (pTreeModel1) delete pTreeModel1;
	if (pTreeModel3) delete pTreeModel2;
	if (pTreeModel2) delete pTreeModel3;

	if (my_job == J_MAGICIAN) {
		if (!pMagicainSkill1) {
			pMagicainSkill1 = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel1, 0);
			pMagicainSkill1->SetPosition(0.f, -100.f, 0.f);
			pMagicainSkill1->SetScale(100.0f, 100.0f, 100.0f);
		}
		if (!pMagicainSkill2) {
			pMagicainSkill2 = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel2, 0);
			pMagicainSkill2->SetPosition(0.f, -100.f, 0.f);
			pMagicainSkill2->SetScale(30.0f, 30.0f, 30.0f);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::BuildObjects_Raid(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 200); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 50.0f, 8.0f); 
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/Gaia2.raw"), 512, 512, xmf3Scale, xmf4Color);


	m_nHierarchicalGameObjects = 3 + 1 + 4 + 3 + 1 + 1;	// 플레이어3 + 가이아1 + 바닥장판4 + 파도3 + 참격1 + 주변오브젝트 1
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];
	//for (int i = 0; i < m_nHierarchicalGameObjects; ++i) m_ppHierarchicalGameObjects[i] = NULL;

	// Character Model Load
	pBastardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bastard_Warrior.bin", NULL);
	player_anim_cnt = pBastardModel->m_pAnimationSets->m_nAnimationSets;

	pTankerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Mighty_Warrior.bin", NULL);

	pSupporterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Priestess.bin", NULL);
	
	pMagicianModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Wizard_Girl.bin", NULL);

	// Magicial Skill Model Load
	pMagicainSkillModel1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Asteroid_1.bin", NULL);
	pMagicainSkillModel2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Asteroid_2.bin", NULL);

	for (int i = 0; i < GAIA_ROOM - 1; i++) {
		switch (get_job(i)) {
		case J_DILLER: {
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBastardModel, player_anim_cnt);
			m_ppHierarchicalGameObjects[i]->SetPosition(3550.0f, m_pTerrain->GetHeight(3550.0f, 650.0f), 650.0f);
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);

			for (int j = 0; j < player_anim_cnt; ++j) {
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
			}
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			break;
		}
		case J_TANKER: {
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTankerModel, player_anim_cnt);
			m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
			for (int j = 0; j < player_anim_cnt; ++j) {
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
			}
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			break;
		}
		case J_SUPPORTER: {
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSupporterModel, player_anim_cnt);
			m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
			for (int j = 0; j < player_anim_cnt; ++j) {
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
			}
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			break;
		}
		case J_MAGICIAN: {
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicianModel, player_anim_cnt);
			m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
			m_ppHierarchicalGameObjects[i]->SetScale(12.0f, 12.0f, 12.0f);
			for (int j = 0; j < player_anim_cnt; ++j) {
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
			}
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);

			// Skill Model 추가
			CMagicianSKillObject* skill_model1 = new CMagicianSKillObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel1, 0, i);
			skill_model1->skillModel->SetPosition(0, -100, 0);
			skill_model1->skillModel->SetScale(100.0f, 100.0f, 100.0f);
			vMagicianSkillModel1p.push_back(skill_model1);

			CMagicianSKillObject* skill_model2 = new CMagicianSKillObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel2, 0, i);
			skill_model2->skillModel->SetPosition(0, -100, 0);
			skill_model2->skillModel->SetScale(30.0f, 30.0f, 30.0f);
			vMagicianSkillModel2p.push_back(skill_model2);
			break;
		}
		default: {
			m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBastardModel, player_anim_cnt);
			m_ppHierarchicalGameObjects[i]->SetPosition(3550.0f, m_pTerrain->GetHeight(3550.0f, 650.0f), 650.0f);
			m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
			for (int j = 0; j < player_anim_cnt; ++j) {
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
			}
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			break;
		}
		}

		for (int j = 2; j < player_anim_cnt; ++j) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
		}
	}

	CLoadedModelInfo* pGaiaModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Gaia.bin", NULL);
	int gaia_anim_cnt = pGaiaModel->m_pAnimationSets->m_nAnimationSets;

	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGaiaModel, gaia_anim_cnt);
	for (int i = 0; i < gaia_anim_cnt; ++i) {
		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		//m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationTracks[i].m_fSpeed = 0.5f;
	}
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(0,true);
	m_ppHierarchicalGameObjects[3]->SetPosition(3500.0f, m_pTerrain->GetHeight(3500.0f, 650.0f), 650.0f);
	m_ppHierarchicalGameObjects[3]->SetScale(20.0f, 20.0f, 20.0f);
	if (pGaiaModel) delete pGaiaModel;

	// 2번 부터 ANIMATION_TYPE_ONCE
	for (int i = 2; i < gaia_anim_cnt; ++i) {
		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i]->m_nType = ANIMATION_TYPE_ONCE;
	}

	// 장판
	CLoadedModelInfo* pCircleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Boss_Floor.bin", NULL);
	for (int i = 4; i < 8; i++) {
		m_ppHierarchicalGameObjects[i] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCircleModel, 0);
		//m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fSpeed = 1.0f;
		m_ppHierarchicalGameObjects[i]->SetPosition(3200.0f, m_pTerrain->GetHeight(3200.0f, 700.0f), 700.0f);
		//m_ppHierarchicalGameObjects[i]->Rotate(-90, 0, 0.0f);
		m_ppHierarchicalGameObjects[i]->SetScale(40.0f, 40.0f, 40.0f);
	}

	// 파도
	CLoadedModelInfo* pWaveModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Boss_Wave.bin", NULL);
	for (int i = 8; i < 11; i++) {
		m_ppHierarchicalGameObjects[i] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWaveModel, 1);
		m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fSpeed = 1.0f;
		m_ppHierarchicalGameObjects[i]->SetPosition(0.f, -100.f, 0.f);
		m_ppHierarchicalGameObjects[i]->SetScale(500.0f, 450.0f, 450.0f);
	}

	// 참격
	CLoadedModelInfo* pSlashModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Boss_Bird.bin", NULL);
	m_ppHierarchicalGameObjects[11] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSlashModel, 1);
	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fSpeed = 0.2f;
	m_ppHierarchicalGameObjects[11]->SetPosition(0.0f, -100.f, 0.0f);
	m_ppHierarchicalGameObjects[11]->SetScale(55.0f, 35.0f, 35.0f);

	// 신전기둥
	CLoadedModelInfo* pRaidObjModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Raid_Model.bin", NULL);
	m_ppHierarchicalGameObjects[12] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pRaidObjModel, 0);
	m_ppHierarchicalGameObjects[12]->SetPosition(0.0f, -100.0f, 0.0f);
	m_ppHierarchicalGameObjects[12]->SetScale(10.0f, 10.0f, 10.0f);

	if (pRaidObjModel) delete pRaidObjModel;
	if (pCircleModel) delete pCircleModel;
	if (pWaveModel) delete pWaveModel;
	if (pSlashModel) delete pSlashModel;

	for (int i = 0; i < 4; i++) get_raid_initialize_position(m_ppHierarchicalGameObjects[i], i);

	if (my_job == J_MAGICIAN) {
		if (!pMagicainSkill1) {
			pMagicainSkill1 = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel1, 0);
			pMagicainSkill1->SetPosition(0.f, -100.f, 0.f);
			pMagicainSkill1->SetScale(100.0f, 100.0f, 100.0f);
		}
		if (!pMagicainSkill2) {
			pMagicainSkill2 = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel2, 0);
			pMagicainSkill2->SetPosition(0.f, -100.f, 0.f);
			pMagicainSkill2->SetScale(30.0f, 30.0f, 30.0f);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::BuildObjects_Login(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 150); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 50.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/Gaia2.raw"), 512, 512, xmf3Scale, xmf4Color);


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;

	if (pBastardModel) delete pBastardModel;
	if (pTankerModel) delete pTankerModel;
	if (pSupporterModel) delete pSupporterModel;
	if (pMagicianModel) delete pMagicianModel;

	if (vMagicianSkillModel1p.size() != 0) {
		vector<CMagicianSKillObject*>::iterator iter = vMagicianSkillModel1p.begin();
		vector<CMagicianSKillObject*>::iterator enditer = vMagicianSkillModel1p.end();
		for (; iter != enditer; ++iter) {
			delete (*iter);
		}
		vMagicianSkillModel1p.clear();
	}
	if (vMagicianSkillModel2p.size() != 0) {
		vector<CMagicianSKillObject*>::iterator iter = vMagicianSkillModel2p.begin();
		vector<CMagicianSKillObject*>::iterator enditer = vMagicianSkillModel2p.end();
		for (; iter != enditer; ++iter) {
			delete (*iter);
		}
		vMagicianSkillModel2p.clear();
	}

	if (pMagicainSkill1) delete pMagicainSkill1;
	if (pMagicainSkill2) delete pMagicainSkill2;

	if (pMagicainSkillModel1) delete pMagicainSkillModel1;
	if (pMagicainSkillModel2) delete pMagicainSkillModel2;

}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[11];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 3; //t3: gtxtTerrainRoadTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[16];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256占쏙옙 占쏙옙占?
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	if(Login_OK)
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
		case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
		case RESOURCE_TEXTURE2D_ARRAY:
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
			break;
		case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
			d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
			d3dShaderResourceViewDesc.Buffer.NumElements = 0;
			d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
			d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		XMFLOAT3 Light_pos = m_pPlayer->GetPosition();
		m_pLights[0].m_xmf3Position = XMFLOAT3(Light_pos.x, Light_pos.y, Light_pos.z);
		
		m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);

	// other Modle Position Render
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			if (Login_OK) {
				if (InDungeon) {
					Raid_Render(pd3dCommandList, pCamera, i);
				}
				else {
					OpenWorld_Render(pd3dCommandList, pCamera, i);
				}
			}
			else {
				Login_Render(pd3dCommandList, pCamera, i);
			}
		}
	}

	// Magician Skill(메테오)(My) Position Look vector
	if(pMagicainSkill1){
		// 포지션 잡기
		if (pMagicainSkill1->GetPosition().y != -100) { // Skill active
			if (pMagicainSkill1->GetPosition().y <= 
				m_pTerrain->GetHeight(pMagicainSkill1->GetPosition().x, pMagicainSkill1->GetPosition().z)) {
				pMagicainSkill1->SetPosition(0, -100, 0);
			}
			else {
				XMFLOAT3 temp_pos = pMagicainSkill1->GetPosition();
				XMFLOAT3 temp_look = pMagicainSkill1->GetLook();
				temp_pos.x += m_fElapsedTime * 40 * temp_look.x;
				temp_pos.y += m_fElapsedTime * 40 * temp_look.y;
				temp_pos.z += m_fElapsedTime * 40 * temp_look.z;
				pMagicainSkill1->SetPosition(temp_pos);
			}
		}
		else {
			if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable && 
				m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition <= 0.1f) {
				XMFLOAT3 temp_pos = m_pPlayer->GetPosition();
				temp_pos.y += 50;
				pMagicainSkill1->SetPosition(temp_pos);
				XMFLOAT3 temp_look = m_pPlayer->GetLook();
				temp_look.y = -0.75;
				temp_look = Vector3::Normalize(temp_look);
				pMagicainSkill1->SetLook(temp_look);
			}
		}

		pMagicainSkill1->Animate(m_fElapsedTime);
		pMagicainSkill1->UpdateTransform(NULL);
		pMagicainSkill1->Render(pd3dCommandList, pCamera);
	}

	// Magician Skill(파이어볼)(My) Position Look vector
	if (pMagicainSkill2) {
		// 포지션 잡기
		if (pMagicainSkill2->GetPosition().y != -100) { // Skill active
			if (chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - MagicainSkill2_start_time).count() >= 400) {
				pMagicainSkill2->SetPosition(0, -100, 0);
			}
			else {
				XMFLOAT3 temp_pos = pMagicainSkill2->GetPosition();
				XMFLOAT3 temp_look = pMagicainSkill2->GetLook();
				temp_pos.x += m_fElapsedTime * temp_look.x * 400;
				temp_pos.y += m_fElapsedTime * temp_look.y * 400;
				temp_pos.z += m_fElapsedTime * temp_look.z * 400;
				pMagicainSkill2->SetPosition(temp_pos);
			}
		}
		else {
			if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable &&
				m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[5]->m_fPosition <= 0.1f) {
				XMFLOAT3 temp_pos = m_pPlayer->GetPosition();
				temp_pos.y += 10;
				pMagicainSkill2->SetPosition(temp_pos);
				XMFLOAT3 temp_look = m_pPlayer->GetLook();
				temp_look = Vector3::Normalize(temp_look);
				pMagicainSkill2->SetLook(temp_look);
				MagicainSkill2_start_time = chrono::system_clock::now();
			}
		}

		pMagicainSkill2->Animate(m_fElapsedTime);
		pMagicainSkill2->UpdateTransform(NULL);
		pMagicainSkill2->Render(pd3dCommandList, pCamera);
	}

	// Magician Skill(메테오)(Other) Position Look vector
	for (int j = 0; j < vMagicianSkillModel1p.size(); j++) {
		if (!InDungeon) {
			if (mPlayer[vMagicianSkillModel1p[j]->_id - (3 + MAX_NPC)]->GetUse() == false) {
				vMagicianSkillModel1p.erase(vMagicianSkillModel1p.begin() + j);
				j--;
				continue;
			}
		}

		// 포지션 잡기
		if (vMagicianSkillModel1p[j]->skillModel->GetPosition().y != -100) { // Skill active
			if (vMagicianSkillModel1p[j]->skillModel->GetPosition().y <= 
				m_pTerrain->GetHeight(vMagicianSkillModel1p[j]->skillModel->GetPosition().x, vMagicianSkillModel1p[j]->skillModel->GetPosition().z)) {
				vMagicianSkillModel1p[j]->skillModel->SetPosition(0, -100, 0);
			}
			else {
				XMFLOAT3 temp_pos = vMagicianSkillModel1p[j]->skillModel->GetPosition();
				XMFLOAT3 temp_look = vMagicianSkillModel1p[j]->skillModel->GetLook();
				temp_pos.x += m_fElapsedTime * 40 * temp_look.x;
				temp_pos.y += m_fElapsedTime * 40 * temp_look.y;
				temp_pos.z += m_fElapsedTime * 40 * temp_look.z;
				vMagicianSkillModel1p[j]->skillModel->SetPosition(temp_pos);
			}
		}
		else {
			if (m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable &&
				m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition <= 0.1f) {
				XMFLOAT3 temp_pos = m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->GetPosition();
				temp_pos.y += 50;
				vMagicianSkillModel1p[j]->skillModel->SetPosition(temp_pos);
				XMFLOAT3 temp_look = m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->GetLook();
				temp_look.y = -0.75;
				temp_look = Vector3::Normalize(temp_look);
				vMagicianSkillModel1p[j]->skillModel->SetLook(temp_look);
			}
		}

		vMagicianSkillModel1p[j]->Animate(m_fElapsedTime);
		vMagicianSkillModel1p[j]->UpdateTransform(NULL);
		vMagicianSkillModel1p[j]->Render(pd3dCommandList, pCamera);
	}

	// Magician Skill(파이어볼)(Other) Position Look vector
	for (int j = 0; j < vMagicianSkillModel2p.size(); j++) {
		if (!InDungeon) {
			if (mPlayer[vMagicianSkillModel2p[j]->_id - (3 + MAX_NPC)]->GetUse() == false) {
				vMagicianSkillModel2p.erase(vMagicianSkillModel2p.begin() + j);
				j--;
				continue;
			}
		}

		// 포지션 잡기
		if (vMagicianSkillModel2p[j]->skillModel->GetPosition().y != -100) { // Skill active
			if (chrono::duration_cast<chrono::milliseconds>( chrono::system_clock::now() - vMagicianSkillModel2p[j]->start_time).count() >= 400) {
				vMagicianSkillModel2p[j]->skillModel->SetPosition(0, -100, 0);
			}
			else {
				XMFLOAT3 temp_pos = vMagicianSkillModel2p[j]->skillModel->GetPosition();
				XMFLOAT3 temp_look = vMagicianSkillModel2p[j]->skillModel->GetLook();
				temp_pos.x += m_fElapsedTime * temp_look.x * 400;
				temp_pos.y += m_fElapsedTime * temp_look.y * 400;
				temp_pos.z += m_fElapsedTime * temp_look.z * 400;
				vMagicianSkillModel2p[j]->skillModel->SetPosition(temp_pos);
			}
		}
		else {
			if (m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable &&
				m_ppHierarchicalGameObjects[vMagicianSkillModel1p[j]->_id]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[5]->m_fPosition <= 0.1f) {
				XMFLOAT3 temp_pos = m_ppHierarchicalGameObjects[vMagicianSkillModel2p[j]->_id]->GetPosition();
				temp_pos.y += 10;

				vMagicianSkillModel2p[j]->skillModel->SetPosition(temp_pos);
				XMFLOAT3 temp_look = m_ppHierarchicalGameObjects[vMagicianSkillModel2p[j]->_id]->GetLook();
				temp_look = Vector3::Normalize(temp_look);
				vMagicianSkillModel2p[j]->skillModel->SetLook(temp_look);
				vMagicianSkillModel2p[j]->start_time = chrono::system_clock::now();
			}
		}

		vMagicianSkillModel2p[j]->Animate(m_fElapsedTime);
		vMagicianSkillModel2p[j]->UpdateTransform(NULL);
		vMagicianSkillModel2p[j]->Render(pd3dCommandList, pCamera);
	}
}

void CScene::OpenWorld_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int i)
{
	// Monster Position Look vector
	if (i >= 3 && i < 3+MAX_NPC) {
		get_object_information(m_ppHierarchicalGameObjects[i], NPC_ID_START + (i - 3));
		m_ppHierarchicalGameObjects[i]->SetPosition(
			XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
				m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
				m_ppHierarchicalGameObjects[i]->GetPosition().z
			)
		);
	}

	// Other Player Position Look vector
	if (i >= 3 + MAX_NPC && i < 3 + MAX_NPC + NUM_PLAYER) {
		if (mPlayer[i - (3+MAX_NPC)]->GetUse() == true && m_ppHierarchicalGameObjects[i]->GetPosition().x == 0.f) {
			switch (mPlayer[i- (3 + MAX_NPC)]->m_job) {
			case J_DILLER: {
				delete m_ppHierarchicalGameObjects[i];
				//m_ppHierarchicalGameObjects[i] = nullptr;
				m_ppHierarchicalGameObjects[i] = new CMonsterObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBastardModel, player_anim_cnt);
				m_ppHierarchicalGameObjects[i]->SetPosition(3550.0f, m_pTerrain->GetHeight(3550.0f, 650.0f), 650.0f);
				m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);

				for (int j = 0; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
				}
				for (int j = 2; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
				}
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
				break;
			}
			case J_TANKER: {
				delete m_ppHierarchicalGameObjects[i];
				m_ppHierarchicalGameObjects[i] = new CMonsterObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTankerModel, player_anim_cnt);
				m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
				m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
				for (int j = 0; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
				}
				for (int j = 2; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
				}
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
				break;
			}
			case J_SUPPORTER: {
				delete m_ppHierarchicalGameObjects[i];
				m_ppHierarchicalGameObjects[i] = new CMonsterObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSupporterModel, player_anim_cnt);
				m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
				m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
				for (int j = 0; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
				}
				for (int j = 2; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
				}
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
				break;
			}
			case J_MAGICIAN: {
				delete m_ppHierarchicalGameObjects[i];
				m_ppHierarchicalGameObjects[i] = new CMonsterObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicianModel, player_anim_cnt);
				m_ppHierarchicalGameObjects[i]->SetPosition(3600.0f, m_pTerrain->GetHeight(3600.0f, 650.0f), 650.0f);
				m_ppHierarchicalGameObjects[i]->SetScale(12.0f, 12.0f, 12.0f);
				for (int j = 0; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 1.0f;
				}
				for (int j = 2; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
				}
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);

				// Skill Model 추가
				CMagicianSKillObject* skill_model1 = new CMagicianSKillObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel1, 0, i);
				skill_model1->skillModel->SetPosition(0, -100, 0);
				skill_model1->skillModel->SetScale(100.0f, 100.0f, 100.0f);
				vMagicianSkillModel1p.push_back(skill_model1);

				CMagicianSKillObject* skill_model2 = new CMagicianSKillObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMagicainSkillModel2, 0, i);
				skill_model2->skillModel->SetPosition(0, -100, 0);
				skill_model2->skillModel->SetScale(20.0f, 20.0f, 20.0f);
				vMagicianSkillModel2p.push_back(skill_model2);

				break;
			}

			default: {
				delete m_ppHierarchicalGameObjects[i];
				m_ppHierarchicalGameObjects[i] = new CMonsterObject(m_pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBastardModel, player_anim_cnt);
				m_ppHierarchicalGameObjects[i]->SetPosition(3550.0f, m_pTerrain->GetHeight(3550.0f, 650.0f), 650.0f);
				m_ppHierarchicalGameObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
				for (int j = 0; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[j].m_fSpeed = 0.5f;
				}
				for (int j = 2; j < player_anim_cnt; ++j) {
					m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[j]->m_nType = ANIMATION_TYPE_ONCE;
				}
				m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
				break;
			}
			}
		}
		get_player_information(m_ppHierarchicalGameObjects[i], i - (3+MAX_NPC));
		m_ppHierarchicalGameObjects[i]->SetPosition(
			XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
				m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
				m_ppHierarchicalGameObjects[i]->GetPosition().z
			)
		);
	}
	
	// Town Npc Position Loock vector
	TownNpc::UpdateTime(m_fElapsedTime);
	if (i >= 3 + MAX_NPC + NUM_PLAYER && i < 3 + MAX_NPC + NUM_PLAYER + NUM_TOWN_NPC) {
		int temp_i = i - (3 + MAX_NPC + NUM_PLAYER);
		m_ppHierarchicalGameObjects[i]->SetPosition(TownNpc::UpdatePosition(temp_i));
		m_ppHierarchicalGameObjects[i]->SetPosition(
			XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
				m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
				m_ppHierarchicalGameObjects[i]->GetPosition().z
			)
		);
		m_ppHierarchicalGameObjects[i]->SetLook(TownNpc::UpdateLook(temp_i));
		if (m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[1].m_bEnable) {
		}
	}



	m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);

	if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
	m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);

}

void CScene::Raid_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int i)
{
	if (i < 3) {
		get_raid_information(m_ppHierarchicalGameObjects[i], i);
		m_ppHierarchicalGameObjects[i]->SetPosition(
			XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
				m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
				m_ppHierarchicalGameObjects[i]->GetPosition().z
			)
		);
		// 사망시
		if (m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
			m_ppHierarchicalGameObjects[i]->Animate(0.005);
		}
		else m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
	}

	else if (i >= 4 && i < 8) {		// 장판
		if (m_gaiaPattern.pattern_on[0]) {
			m_ppHierarchicalGameObjects[i]->SetPosition(m_gaiaPattern.pattern_one[i-4]);
			m_ppHierarchicalGameObjects[i]->SetPosition(
				XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
					m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z) + 10.0f - (30.0f - circle_time),
					m_ppHierarchicalGameObjects[i]->GetPosition().z
				)
			);
			
			if (circle_time < 30.0f)
				m_ppHierarchicalGameObjects[i]->Rotate(0.0f, circle_time * 6.0f, 0.0f);

			if (circle_time >= 30.0f) circle_time = 30.0f;
			else if (circle_time >= 10.0f) circle_time += 0.8f;
			else circle_time += 0.1f;

			
			m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fSpeed = 1.3f;
			m_isIdle = false;
			// 3, 3 -> 3번 오브젝트(가이아)의 3번 애니메이션을 True로
			SetAnimationEnableTrue(3, 3, 0.7f);
			

		}
		else {
			m_ppHierarchicalGameObjects[i]->SetPosition(XMFLOAT3(0, -100, 0));
			circle_time = 0.0f;
			//m_isIdle = true;
		}
	}
	else if (i >= 8 && i < 11) {	// 파도
		if (m_gaiaPattern.pattern_on[1]) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_ppHierarchicalGameObjects[i]->Animate(0.01f);
			m_ppHierarchicalGameObjects[i]->SetPosition(m_gaiaPattern.pattern_two[i-8]);
			m_ppHierarchicalGameObjects[i]->SetPosition(
				XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
					m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
					m_ppHierarchicalGameObjects[i]->GetPosition().z
				)
			);

			m_ppHierarchicalGameObjects[i]->SetLook(m_gaiaPattern.pattern_two_look);
		

			m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_fSpeed = 1.2f;
			m_isIdle = false;
			SetAnimationEnableTrue(3, 4, 1.0f);
			
		}

		else {
			m_ppHierarchicalGameObjects[i]->SetPosition(XMFLOAT3(0, -100, 0));
			//m_isIdle = true;
		}
	}
	else if (i == 11) { // 참격
		if (m_gaiaPattern.pattern_on[4]) {
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_ppHierarchicalGameObjects[i]->Animate(0.1f);
			m_ppHierarchicalGameObjects[i]->SetPosition(m_gaiaPattern.pattern_five);
			m_ppHierarchicalGameObjects[i]->SetPosition(
				XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
					m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z) + 10.0f,
					m_ppHierarchicalGameObjects[i]->GetPosition().z
				)
			);
			m_ppHierarchicalGameObjects[i]->SetLook(m_gaiaPattern.pattern_five_look);

			slash_time += 1.0f;

			m_ppHierarchicalGameObjects[i]->Rotate(0.0f, 0.0f, slash_time * 24.0f);

			m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationTracks[3].m_fSpeed = 0.7f;
			m_isIdle = false;
			SetAnimationEnableTrue(3, 2, 0.7f);
		}
		else {
			m_ppHierarchicalGameObjects[i]->SetPosition(XMFLOAT3(0, -100, 0));
			slash_time = 0.0f;
			//m_isIdle = true;
		}
	}

	else if (i == 12) {
		m_ppHierarchicalGameObjects[i]->SetPosition(2037.5f, m_pTerrain->GetHeight(2037.5f, 2112.3f), 2112.3f);
	}

	else if (i == 3) {
		get_gaia_information(m_ppHierarchicalGameObjects[i]);
		if (get_combat_id_hp() <= 0) {
			m_ppHierarchicalGameObjects[i]->SetPosition(0, -100, 0);
		}
		else {
			m_ppHierarchicalGameObjects[i]->SetPosition(
				XMFLOAT3(m_ppHierarchicalGameObjects[i]->GetPosition().x,
					m_pTerrain->GetHeight(m_ppHierarchicalGameObjects[i]->GetPosition().x, m_ppHierarchicalGameObjects[i]->GetPosition().z),
					m_ppHierarchicalGameObjects[i]->GetPosition().z
				)
			);
		}

		if (m_isIdle) {
			SetAnimationEnableTrue(3, 0);
		}
		m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
	}

	if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
	m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
}
void CScene::Login_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int i)
{

}
void CScene::SetAnimationEnableTrue(int objIndex, int animIndex, int speed)
{
	int max_anim = m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->m_nAnimationTracks;
	SetAnimationPositionZero(objIndex, animIndex);
	for (int i = 0; i < max_anim; ++i) {
		if (i == animIndex) {
			m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->SetTrackEnable(i, true);

			//-사운드
			continue;
		}
		m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	if (IsAnimationEnd(objIndex/*3*/, animIndex/*2*/)) {
		m_isIdle = true;
		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[animIndex]->m_fPosition = 0.0f;
		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(animIndex, false);
		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	}


}

// nAnim을 제외한 나머지를 제로로 만드렁
void CScene::SetAnimationPositionZero(int objIndex, int animIndex)
{
	int max_anim = m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->m_nAnimationTracks;

	for (int i = 0; i < max_anim; ++i) {
		if (i == animIndex) continue;
		m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i]->m_fPosition = 0.0f;
	}
}

bool CScene::IsAnimationEnd(int objIndex, int animIndex)
{
	float anim_now = m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[animIndex]->m_fPosition;
	float anim_end = m_ppHierarchicalGameObjects[objIndex]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[animIndex]->m_fLength;
	
	return (anim_end - anim_now <= 0.05f);
}

void CScene::SetTreePosition(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int start, int end)
{
	ifstream in{ "tree_position.txt" };
	if (!in)
	{
		cout << "tree_position.txt 파일을 열 수 없습니다." << endl;
		exit(0);
	}

	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float> uidScale(7.0f, 14.0f);
	uniform_real_distribution<float> uidRotate(0.0f, 180.0f);
	uniform_int_distribution<> uidModel(0, 2);

	XMFLOAT3 xmf3Pos;
	int cnt = start;
	int nModel = 0;
	while (cnt <= end) {
		in >> xmf3Pos.x;
		in >> xmf3Pos.y;
		in >> xmf3Pos.z;
		nModel = uidModel(dre);
		switch (nModel) {
		case 0:
			m_ppHierarchicalGameObjects[cnt] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel1, 0);
			break;
		case 1:
			m_ppHierarchicalGameObjects[cnt] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel2, 0);
			break;
		case 2:
			m_ppHierarchicalGameObjects[cnt] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel3, 0);
			break;
		default:
			m_ppHierarchicalGameObjects[cnt] = new CCastleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pTreeModel1, 0);
			break;
		}
		m_ppHierarchicalGameObjects[cnt]->SetPosition(xmf3Pos.x, m_pTerrain->GetHeight(xmf3Pos.x, xmf3Pos.z), xmf3Pos.z);
		m_ppHierarchicalGameObjects[cnt]->SetScale(uidScale(dre), uidScale(dre), uidScale(dre));
		m_ppHierarchicalGameObjects[cnt]->Rotate(0.0f, uidRotate(dre), 0.0f);
		++cnt;
	}


}

