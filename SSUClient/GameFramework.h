#pragma once

#define FRAME_BUFFER_WIDTH		1800 //640
#define FRAME_BUFFER_HEIGHT		900 //480
#define UICOUNT 44

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Network.h"

extern bool shoot;
extern  XMFLOAT3 hp_pos;
extern XMFLOAT3 POS_PLAYER;

extern wstring Chatting_Str;
extern wstring Invite_Str;
extern bool Chatting_On;
extern bool Mouse_On;

extern int effect_x;
extern int effect_y;
extern int effect_z;
extern bool hit_check;

extern int nnn;

extern CCamera* m_pCamera;
extern wstring ID_Str;
extern wstring PASSWORD_Str;
extern wstring JOIN_ID_Str;
extern wstring JOIN_PASSWORD_Str;
extern wstring JOIN_NICKNAME_Str;
extern wstring JOIN_JOB_Str;
extern wstring JOIN_ELEMENT_Str;

extern bool point_light_bool;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
	void BuildObjects_login();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void Release_OpenWorld_Object();
	void Release_InDungeon_Object();
	void Release_Login_Object();
	void Create_OpenWorld_Object();
	void Create_InDungeon_Object();
	void Create_Login_Object();

	bool RaySphereIntersect(XMFLOAT3 rayOrigin, XMFLOAT3 rayDirection, float radius);
	bool TestIntersection(int mouseX, int mouseY, CPlayer* obj);

	void Login_Check_And_Build();

	int m_nDamageLayer = 0;
private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;
	CScene						*m_pRaid_Scene = NULL;
	CScene						*m_pLogin_Scene = NULL;

	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	UILayer** m_ppUILayer = NULL;

};

