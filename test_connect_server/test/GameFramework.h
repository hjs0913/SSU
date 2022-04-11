#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480
#define UICOUNT 12

//#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Network.h"
#include "../../RealServer/Server/protocol.h"

extern bool shoot;
extern  XMFLOAT3 hp_pos;
extern XMFLOAT3 POS_PLAYER; 

extern wstring Chatting_Str;
extern bool Chatting_On;
extern bool Mouse_On;

extern int effect_x;
extern int effect_y;
extern int effect_z;
extern bool hit_check;

extern int nnn;

extern CCamera* m_pCamera;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd, HWND hPartyWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreatePartySwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void UpdateUI(int i);
	bool RaySphereIntersect(XMFLOAT3 rayOrigin, XMFLOAT3 rayDirection, float radius)
	{
		// a, b 및 c 계수를 계산합니다.
		float a = (rayDirection.x * rayDirection.x) + (rayDirection.y * rayDirection.y) + (rayDirection.z * rayDirection.z);
		float b = ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) + (rayDirection.z * rayOrigin.z)) * 2.0f;
		float c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) + (rayOrigin.z * rayOrigin.z)) - (radius * radius);

		// 결과값을 얻는다
		float discriminant = (b * b) - (4 * a * c);

		// 결과값이 음수이면 피킹 선이 구를 벗어난 것입니다. 그렇지 않으면 구를 선택합니다.
		if (discriminant < 0.0f)
		{
			return false;
		}

		return true;
	};

	bool TestIntersection(int mouseX, int mouseY, CGameObject* obj);

	void OnMouseMove(WPARAM btnState, int x, int y);
	
	

	void Release_OpenWorld_Object();
	void Release_InDungeon_Object();

	void Create_OpenWorld_Object();
	void Create_InDungeon_Object();

private:
	/////////////////
	/////////////////


	float cameradis = 1.0f;
	float savedis = 1.0f;

	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 
	HWND						m_Partyjoin_hWnd;


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
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

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

	CPlayer						*m_pPlayer = NULL;
	

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[50];

	UILayer						** m_ppUILayer = NULL;	
};

class cRay :public CGameFramework 
{
public:
	cRay() {
	
		m_vOriginal = {0.0f, 0.0f, 0.0f};
		 m_vDirection = { 0.0f, 0.0f, 0.0f };
		 m_eRaySpace = E_NONE;
	}
	~cRay() {  }

	enum eRaySpace
	{
		E_NONE, E_VIEW, E_WORLD, E_LOCAL
	};


	cRay cRay::RayAtViewspace(int nScreenX, int nScreenY)
	{

		D3D12_VIEWPORT viewport;

	    viewport = m_pCamera->GetViewport();  //문제
		
		XMFLOAT4X4 MatProjection;
	
     	MatProjection = m_pCamera->GetProjectionMatrix();  //이거도문제 
		
		cRay r;
		r.m_vDirection.x = ((2.0f * nScreenX) / viewport.Width - 1.0f) / MatProjection._11;
		r.m_vDirection.y = ((-2.0f * nScreenY) / viewport.Height + 1.0f) / MatProjection._22;
		r.m_vDirection.z = -1.0f;
		r.m_eRaySpace = E_VIEW;
		return r;
	};


	cRay cRay::RayAtWorldSpace(int nScreenX, int nScreenY)
	{
		cRay r = cRay::RayAtViewspace(nScreenX, nScreenY);
		


		XMFLOAT4X4 f_matView, f_matInvView;
		XMMATRIX matView, matInvView;
		XMVECTOR DE;
	
		f_matView = m_pCamera->GetProjectionMatrix(); //이거도문제 
		matView = XMLoadFloat4x4(&f_matView);

		DE = XMMatrixDeterminant(matView);
		matInvView = XMMatrixInverse(&DE, matView);

		XMStoreFloat3(&r.m_vOriginal, XMVector3TransformCoord(XMVectorSet(r.m_vOriginal.x, r.m_vOriginal.y, r.m_vOriginal.z, 0.0f), matInvView));
		XMStoreFloat3(&r.m_vDirection, XMVector3TransformNormal(XMVectorSet(r.m_vDirection.x, r.m_vDirection.y, r.m_vDirection.z, 0.0f), matInvView));
		XMStoreFloat3(&r.m_vDirection, XMVector3Normalize(XMVectorSet(r.m_vDirection.x, r.m_vDirection.y, r.m_vDirection.z, 0.0f)));

		r.m_eRaySpace = E_WORLD;




		return r;
	};

	bool cRay::isPicked(CGameObject* obj)
	{
		//cout << obj->GetPosition().x << endl;
	 //  cout << obj->GetPosition().y << endl;
	//	cout << obj->GetPosition().z << endl;
		cRay r = (*this);
		


		XMMATRIX matInvWorld;
	
		XMFLOAT4X4 f_matInvWorld;
	
		matInvWorld = XMMatrixIdentity(); //단위행렬 생성 
		
		XMStoreFloat4x4(&f_matInvWorld, matInvWorld);
		f_matInvWorld._41 = -(obj->GetPosition().x);
		f_matInvWorld._42 = -(obj->GetPosition().y);
		f_matInvWorld._43 = -(obj->GetPosition().z);

		matInvWorld = XMLoadFloat4x4(&f_matInvWorld);

		XMStoreFloat3(&r.m_vOriginal, XMVector3TransformCoord(XMVectorSet(r.m_vOriginal.x, r.m_vOriginal.y, r.m_vOriginal.z, 0.0f), matInvWorld));
		XMStoreFloat3(&r.m_vDirection, XMVector3TransformNormal(XMVectorSet(r.m_vDirection.x, r.m_vDirection.y, r.m_vDirection.z, 0.0f), matInvWorld));



		XMVECTOR f_dir;
		f_dir = XMLoadFloat3(&r.m_vDirection);


		XMVECTOR f_ori;
		f_ori = XMLoadFloat3(&r.m_vOriginal);

		XMVECTOR vv = XMVector3Dot(f_dir, f_dir);
		XMVECTOR qv = XMVector3Dot(f_ori, f_dir);
		XMVECTOR qq = XMVector3Dot(f_ori, f_ori);
		float rr = 2500;

		XMFLOAT3 vv2;  XMStoreFloat3(&vv2, vv);
		XMFLOAT3 qv2;  XMStoreFloat3(&qv2, qv);
		XMFLOAT3 qq2;  XMStoreFloat3(&qq2, qq);

		float a = vv2.x + vv2.y + vv2.z;
		float b = qv2.x + qv2.y + qv2.z;
		float c = qq2.x + qq2.y + qq2.z;

		cout << a << " " << b << " " << c << endl;
		float discriminant = (b * b) - (4 * a * c);
		if (discriminant < 0.0f)
			return false;
		return true;
		//if (b * b - a * (c - rr) >= 0)
		//	return true;
		//else
		//	return false;
	}

protected:
	XMFLOAT3 m_vOriginal;
	XMFLOAT3 m_vDirection;
	eRaySpace m_eRaySpace;



};