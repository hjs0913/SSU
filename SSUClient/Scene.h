//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include "SoundManager.h"

#define MAX_LIGHTS						16 
#define MAX_MATERIAL					8

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void BuildObjects_Raid(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildObjects_Login(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);
	void OpenWorld_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, int i = 0);
	void Raid_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, int i = 0);
	void Login_Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, int i = 0);
	void SetTreePosition(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int start, int end);
	void ReleaseUploadBuffers();

	CPlayer								*m_pPlayer = NULL;

	void SetAnimationEnableTrue(int nObject, int nAnim, int speed = 1.0f);
	// nAnim������ �������� ���η� �����
	void SetAnimationPositionZero(int nObject, int nAnim);
	bool IsAnimationEnd(int nObject, int nAnim);

protected:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap			*m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	float								m_fElapsedTime = 0.0f;

	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

	int									m_nHierarchicalGameObjects = 0;
	CGameObject							**m_ppHierarchicalGameObjects = NULL;

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;
	//-----------------------------------------
	CMaterial* m_pMaterial = NULL;
	ID3D12Resource* m_pd3dcbMaterials = NULL;
	CMaterial* m_ppMaterials = NULL;
	//-----------------------------------------

	ID3D12Device						*m_pd3dDevice = NULL;

	// character model
	CLoadedModelInfo					*pBastardModel = NULL;
	CLoadedModelInfo					*pTankerModel = NULL;
	CLoadedModelInfo					*pSupporterModel = NULL;
	CLoadedModelInfo					*pMagicianModel = NULL;

	// magicial Skill model(1 : ���׿�, 2 : ���̾)
	CLoadedModelInfo					*pMagicainSkillModel1 = NULL;
	CLoadedModelInfo					*pMagicainSkillModel2 = NULL;

	// My magical Skill
	CGameObject							*pMagicainSkill1 = NULL;
	CGameObject							*pMagicainSkill2 = NULL;
	chrono::system_clock::time_point	MagicainSkill2_start_time;

	// Other player magicial Skill
	vector<CMagicianSKillObject*>		vMagicianSkillModel1p;
	vector<CMagicianSKillObject*>		vMagicianSkillModel2p;

	// object model
	CLoadedModelInfo					*pTreeModel1 = NULL;
	CLoadedModelInfo					*pTreeModel2 = NULL;
	CLoadedModelInfo					*pTreeModel3 = NULL;

	// monster model
	CLoadedModelInfo					*pSpiderModel = NULL;			// chicken ���(�ӽ�)
	CLoadedModelInfo					*pRabbitModel = NULL;
	CLoadedModelInfo					*pFrogModel = NULL;
	CLoadedModelInfo					*pMonkeyModel = NULL;
	CLoadedModelInfo					*pWolfModel = NULL;
	CLoadedModelInfo					*pPigModel = NULL;				// tiger ���(�ӽ�)


	int									player_anim_cnt = 0;
	int									monster_anim_cnt = 0;

	float circle_time = 0.0f;
	float slash_time = 0.0f;

	bool m_isIdle = true;

	const int NUM_PLAYER = 30;
	const int NUM_TOWN_NPC = 10;
};