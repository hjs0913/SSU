#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Network.h"
#include "../RealServer/Server/protocol.h"

#define DRAW_SCENE_COLOR				'S'

#define DRAW_SCENE_TEXTURE				'T'
#define DRAW_SCENE_LIGHTING				'L'
#define DRAW_SCENE_NORMAL				'N'
#define DRAW_SCENE_Z_DEPTH				'Z'
#define DRAW_SCENE_DEPTH				'D'

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

	void CreateSwapChainRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd; 

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;
        
	IDXGIFactory4					*m_pdxgiFactory = NULL;
	IDXGISwapChain3					*m_pdxgiSwapChain = NULL;
	ID3D12Device					*m_pd3dDevice = NULL;

	bool							m_bMsaa4xEnable = false;
	UINT							m_nMsaa4xQualityLevels = 0;

	static const UINT				m_nSwapChainBuffers = 2;
	UINT							m_nSwapChainBufferIndex = 0;

	ID3D12Resource					*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap			*m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];

	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap			*m_pd3dDsvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		*m_d3dDepthStencilBufferSRVCPUHandle;

	ID3D12CommandAllocator			*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue				*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList		*m_pd3dCommandList = NULL;

	ID3D12Fence						*m_pd3dFence = NULL;
	UINT64							m_nFenceValues[m_nSwapChainBuffers];
	HANDLE							m_hFenceEvent;

	CGameTimer						m_GameTimer;

	CScene							*m_pScene = NULL;
	
	CPlayer							*m_pPlayer = NULL;
	CCamera							*m_pCamera = NULL;

	CPlayer							*m_pOthers[MAX_USER+MAX_NPC] = { NULL };

	CPlayer							*m_pObstacles[MAX_OBSTACLE] = { NULL };

	// 10.30
	CObjectsShader* m_pObjectShader = NULL;
	
	CLaplacianEdgeShader			*m_pLaplacianEdgeDetectionShader = NULL;

	int								m_nDrawOptions = DRAW_SCENE_COLOR;

	POINT							m_ptOldCursorPos;

	_TCHAR							m_pszFrameRate[50];

	bool							press_keyboard[5]{ false, false, false, false, false };	// w, a, s, d, space
};

