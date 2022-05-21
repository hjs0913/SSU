//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include <iostream>
#include <fstream>
#include <format>
#include <array>

using namespace std;
#define BULLETCNT 100
#define ROOMX 4000
#define ROOMZ 4000

int bulletidx = 1;
float tmp[BULLETCNT];
bool IsFire[BULLETCNT] = {};
bool shoot = false;
bool hit_check = false;

wstring Chatting_Str = L"";
wstring Invite_Str = L"";
wstring Send_str = L"";
wstring Hp_str = L"";
wstring Mp_str = L"";

bool Chatting_On = false;
bool Mouse_On = false;
bool PartyUI_On = false;

int effect_x = 0;
int effect_y = 0;
int effect_z = 0;

MOUSEMOVEPOINT m_mouse;
int m_mouseX = 0;
int m_mouseY = 0;

CCamera* m_pCamera;

bool f4_picking_possible = false;
bool f5_picking_possible = false;
bool f6_picking_possible = false;


CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("SSU ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::RaySphereIntersect(XMFLOAT3 rayOrigin, XMFLOAT3 rayDirection, float radius)
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

bool CGameFramework::TestIntersection(int mouseX, int mouseY, CPlayer* obj)
{


	XMMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
	XMFLOAT4X4 F_projectionMatrix, F_viewMatrix, F_inverseViewMatrix, F_worldMatrix, F_translateMatrix, F_inverseWorldMatrix;
	XMFLOAT3 direction, origin, rayOrigin, rayDirection;
	XMFLOAT3 F3_worldMatrix;
	XMVECTOR V_worldMatrix;

	// 마우스 커서 좌표를 -1에서 +1 범위로 이동합니다
	float pointX = ((2.0f * (float)mouseX) / (float)m_nWndClientWidth) - 1.0f;      //FRAME_BUFFER_WIDTH
	float pointY = (((2.0f * (float)mouseY) / (float)m_nWndClientHeight) - 1.0f) * -1.0f;  //FRAME_BUFFER_HEIGHT

	// 뷰포트의 종횡비를 고려하여 투영 행렬을 사용하여 점을 조정합니다
	F_projectionMatrix = m_pCamera->GetProjectionMatrix();
	projectionMatrix = XMLoadFloat4x4(&F_projectionMatrix);

	XMFLOAT3X3 projectionMatrix4;
	XMStoreFloat3x3(&projectionMatrix4, projectionMatrix);

	pointX = pointX / projectionMatrix4._11;
	pointY = pointY / projectionMatrix4._22;

	// 뷰 행렬의 역함수를 구합니다.
	F_viewMatrix = m_pCamera->GetViewMatrix();
	viewMatrix = XMLoadFloat4x4(&F_viewMatrix);
	inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);

	XMFLOAT3X3 inverseViewMatrix4;
	XMStoreFloat3x3(&inverseViewMatrix4, inverseViewMatrix);



	// 뷰 공간에서 피킹 레이의 방향을 계산합니다.
	direction.x = (pointX * inverseViewMatrix4._11) + (pointY * inverseViewMatrix4._21) + inverseViewMatrix4._31;
	direction.y = (pointX * inverseViewMatrix4._12) + (pointY * inverseViewMatrix4._22) + inverseViewMatrix4._32;
	direction.z = (pointX * inverseViewMatrix4._13) + (pointY * inverseViewMatrix4._23) + inverseViewMatrix4._33;

	// 카메라의 위치 인 picking ray의 원점을 가져옵니다.
	origin = m_pPlayer ->GetPosition();
	
	// 세계 행렬을 가져와 구의 위치로 변환합니다.  //.여기 다시 보자 
	F3_worldMatrix = m_pCamera->GetLookAtPosition();

	///m_xmf3LookAtWorld이걸 써서 수정한 부분 
	//XMFLOAT4X4 worldllook;
//	worldllook._11 = m_pCamera->GetWorld().x; worldllook._12 = 0.0f; worldllook._13 = 0.0f; worldllook._14 = 0.0f;
//	worldllook._21 = 0.0f; worldllook._22 = m_pCamera->GetWorld().y; worldllook._23 = 0.0f; worldllook._24 = 0.0f;
//	worldllook._31 = 0.0f;
// worldllook._32 = 0.0f; worldllook._33 = m_pCamera->GetWorld().z; worldllook._34 = 0.0f;
//	worldllook._41 = 0.0f; worldllook._42 = 0.0f; worldllook._43 = 0.0f; worldllook._44 = 0.0f;
//	worldMatrix = XMLoadFloat4x4(&worldllook);//XMMatrixIdentity();
	worldMatrix = XMMatrixIdentity();
	//translateMatrix = XMMatrixTranslation(obj->vCenter.x, obj->vCenter.y, obj->vCenter.z);
	translateMatrix = XMMatrixTranslation(obj->GetPosition().x, obj->GetPosition().y, obj->GetPosition().z);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	// 이제 번역 된 행렬의 역함수를 구하십시오.
	inverseWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);

	// 이제 광선 원점과 광선 방향을 뷰 공간에서 월드 공간으로 변환합니다.
	XMStoreFloat3(&rayOrigin, XMVector3TransformCoord(XMVectorSet(origin.x, origin.y, origin.z, 0.0f), inverseWorldMatrix));
	XMStoreFloat3(&direction, XMVector3TransformNormal(XMVectorSet(direction.x, direction.y, direction.z, 0.0f), inverseWorldMatrix));

	// 광선 방향을 표준화합니다.
	XMStoreFloat3(&rayDirection, XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 0.0f)));

	// 이제 광선 구 교차 테스트를 수행하십시오.
	if (RaySphereIntersect(rayOrigin, rayDirection, 20.0f) == true)  //피킹 성공
		return true;
	else
		return false;
}
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN: {
			::SetCapture(hWnd);
			::GetCursorPos(&m_ptOldCursorPos);
			POINT CursorPosInClient = m_ptOldCursorPos;
			ScreenToClient(hWnd, &CursorPosInClient);
			if (InvitationCardUI_On) {
				if (CursorPosInClient.y >= 420 && CursorPosInClient.y <= 460) {
					if (CursorPosInClient.x >= 360 && CursorPosInClient.x <= 460) {
						send_party_invitation_reply(1);
						InvitationCardUI_On = false;
					}
					if (CursorPosInClient.x >= 520 && CursorPosInClient.x <= 620) {
						send_party_invitation_reply(0);
						InvitationCardUI_On = false;
					}
				}
			}

			if (PartyUI_On) {
				if (PartyInviteUI_ON) {
					break;
				}

				if (AddAIUI_On) {
					if (CursorPosInClient.y >= 240 && CursorPosInClient.y <= 260) {
						if (CursorPosInClient.x >= 205 && CursorPosInClient.x <= 255) {
							send_party_add_partner(J_DILLER);
							AddAIUI_On = false;
						}
						if (CursorPosInClient.x >= 265 && CursorPosInClient.x <= 315) {
							send_party_add_partner(J_TANKER);
							AddAIUI_On = false;
						}
						if (CursorPosInClient.x >= 325 && CursorPosInClient.x <= 375) {
							send_party_add_partner(J_MAGICIAN);
							AddAIUI_On = false;
						}
						if (CursorPosInClient.x >= 385 && CursorPosInClient.x <= 435) {
							send_party_add_partner(J_SUPPORTER);
							AddAIUI_On = false;
						}
					}
					break;
				}

				if (CursorPosInClient.y >= 360 && CursorPosInClient.y <= 400) {
					if (CursorPosInClient.x >= 140 && CursorPosInClient.x <= 205) {
						if (!party_enter)send_party_room_make();
					}
					if (CursorPosInClient.x >= 215 && CursorPosInClient.x <= 280) {
						if (!party_info_on) break;
						if (party_enter == false) send_party_room_enter_request();
						else send_party_room_quit_request();
					}
					if (CursorPosInClient.x >= 360 && CursorPosInClient.x <= 425) {
						if (party_enter) {
							PartyInviteUI_ON = true;
							Invite_Str = L"";
						}
					}
					if (CursorPosInClient.x >= 435 && CursorPosInClient.x <= 500) {
						if (party_enter) {
							AddAIUI_On = true;
						}
					}
				}
				else {
					if (party_enter) break;
					if (CursorPosInClient.x >= 120 && CursorPosInClient.x <= 300) {
						if (CursorPosInClient.y >= 60 && CursorPosInClient.y <= 100 && robby_cnt >= 1) {
							send_party_room_info_request(party_id_index_vector[0]);
						}
						if (CursorPosInClient.y >= 110 && CursorPosInClient.y <= 150 && robby_cnt >= 2) {
							send_party_room_info_request(party_id_index_vector[1]);
						}
						if (CursorPosInClient.y >= 160 && CursorPosInClient.y <= 200 && robby_cnt >= 3) {
							send_party_room_info_request(party_id_index_vector[2]);
						}
						if (CursorPosInClient.y >= 210 && CursorPosInClient.y <= 250 && robby_cnt >= 4) {
							send_party_room_info_request(party_id_index_vector[3]);
						}
						if (CursorPosInClient.y >= 260 && CursorPosInClient.y <= 300 && robby_cnt >= 5) {
							send_party_room_info_request(party_id_index_vector[4]);
						}
					}
				}
				break;
			}

			if (f4_picking_possible) {
				//cRay r; 
				//r.RayAtWorldSpace(LOWORD(lParam), HIWORD(lParam));
				for (int i = 0; i < MAX_USER; ++i) {  //9615  for (int i = 10615; i < 10795; i++) 
					//if(r.isPicked(m_ppObjects[i])){
					if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, mPlayer[i])) {
						send_picking_skill_packet(2, 0, i);
					//	m_ppObjects[i]->SetMesh(0, pOtherPlayerMesh[1]);  //피킹 확인위해 색상변경 
						f4_picking_possible = false;

					}
				}
			}

			if (f5_picking_possible) {
				for (int i = 0; i < MAX_USER; ++i) { 
					if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, mPlayer[i])) {
						send_picking_skill_packet(0, 0, i);
						f5_picking_possible = false;

					}
				}
			}

			if (f6_picking_possible) {
				for (int i = 0; i < MAX_USER; ++i) { 
					if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, mPlayer[i])) {
						send_picking_skill_packet(1, 0, i);
						f6_picking_possible = false;

					}
				}
			}



		}
		case WM_RBUTTONDOWN:
			::SetCapture(hWnd);
			::GetCursorPos(&m_ptOldCursorPos);
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	char* send_str;
	const wchar_t* temp;
	int len = 0;

	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					::PostQuitMessage(0);
					break;
				case VK_RETURN: {
					if (PartyInviteUI_ON) {
						PartyInviteUI_ON = false;
						len = 1 + Invite_Str.length();
						send_str = new char[len * 4];
						temp = Invite_Str.c_str();
						wcstombs(send_str, temp, MAX_NAME_SIZE);
						send_party_invite(send_str);
						delete send_str;
						break;
					}

					Chatting_On = !Chatting_On;
					if (Chatting_On == false) {
						len = 1 + Chatting_Str.length();
						send_str = new char[len * 4];
						temp = Chatting_Str.c_str();
						wcstombs(send_str, temp, MAX_CHAT_SIZE);
						send_chat_packet(send_str);
						Chatting_Str = L"";
						delete send_str;
					}
					break;
				}
				case VK_F1: {
					switch (my_job)
					{
					case J_DILLER:
						my_job = J_TANKER;
						break;
					case J_TANKER:
						my_job = J_MAGICIAN;
						break;
					case J_MAGICIAN:
						my_job = J_SUPPORTER;
						break;
					case J_SUPPORTER:
						my_job = J_DILLER;
						break;
					default:
						break;
					}
					send_change_job_packet(my_job);
					break;
				}
				case VK_F2: {
					break;
				}
				case VK_F3:
					m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					break;
				case VK_F4:
					f4_picking_possible = true;
					break;
				case VK_F5:
					f5_picking_possible = true;
					break;
				case VK_F6:
					f6_picking_possible = true;
					break;
				case VK_F9:
					ChangeSwapChainState();
					break;
				case 0x50: {	// p key
					if (InDungeon) break;
					if (PartyInviteUI_ON || Chatting_On) break;
					PartyUI_On = !PartyUI_On;
					if (PartyUI_On) send_party_room_packet();
					else {
						party_id_index_vector.clear();
						robby_cnt = 0;
					}
					break;
				}
				default:
					break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				m_GameTimer.Stop();
			else
				m_GameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{
	if (!m_ppUILayer) {
		m_ppUILayer = new UILayer * [UICOUNT];

		// Chatting(0 : read, 1 : Write)
		m_ppUILayer[0] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::White);
		m_ppUILayer[1] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::White);

		// PlayerInfo( 2 : Info, 3: Hp, 4 : Mp)
		m_ppUILayer[2] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::White, D2D1::ColorF::Black);
		m_ppUILayer[3] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);
		m_ppUILayer[4] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Blue, D2D1::ColorF::White);

		// Combat Npc Info( 5: info, 6 : Hp)
		m_ppUILayer[5] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::White, D2D1::ColorF::Black);
		m_ppUILayer[6] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);

		// Boss Hp Info
		m_ppUILayer[7] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::White, D2D1::ColorF::Black);
		m_ppUILayer[8] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);

		// Party Hp Info
		m_ppUILayer[9] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::White, D2D1::ColorF::Black);
		m_ppUILayer[10] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);
		m_ppUILayer[11] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);
		m_ppUILayer[12] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);
		m_ppUILayer[13] = new UIBar(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);

		// Buff UI
		m_ppUILayer[14] = new BuffUI(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Red, D2D1::ColorF::White);

		// Party UI
		m_ppUILayer[15] = new PartyUI(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::White);

		// 파티초대 UI
		m_ppUILayer[16] = new PartyInviteUI(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::Black);

		// 파티 초대장
		m_ppUILayer[17] = new InvitationCardUI(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::DarkGray, D2D1::ColorF::White);

		// AI추가 UI
		m_ppUILayer[18] = new AddAIUI(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::White);

		// Notice AI
		m_ppUILayer[19] = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue, D2D1::ColorF::Gray, D2D1::ColorF::Black);

		m_ppUILayer[0]->setAlpha(0.5, 1.0);
		m_ppUILayer[1]->setAlpha(0.5, 1.0);
		m_ppUILayer[2]->setAlpha(0.3, 1.0);
		m_ppUILayer[3]->setAlpha(0.0, 1.0);
		m_ppUILayer[4]->setAlpha(0.0, 1.0);
		m_ppUILayer[5]->setAlpha(0.3, 1.0);
		m_ppUILayer[6]->setAlpha(0.0, 1.0);
		m_ppUILayer[7]->setAlpha(0.3, 1.0);
		m_ppUILayer[8]->setAlpha(0.0, 1.0);

		m_ppUILayer[9]->setAlpha(0.3, 1.0);
		m_ppUILayer[10]->setAlpha(0.0, 1.0);
		m_ppUILayer[11]->setAlpha(0.0, 1.0);
		m_ppUILayer[12]->setAlpha(0.0, 1.0);
		m_ppUILayer[13]->setAlpha(0.0, 1.0);

		m_ppUILayer[14]->setAlpha(0.0, 1.0);
		m_ppUILayer[15]->setAlpha(0.7, 1.0);
		m_ppUILayer[16]->setAlpha(1.0, 1.0);
		m_ppUILayer[17]->setAlpha(1.0, 1.0);
		m_ppUILayer[18]->setAlpha(1.0, 1.0);
		m_ppUILayer[19]->setAlpha(0.8, 1.0);

		m_ppUILayer[0]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		m_ppUILayer[1]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[2]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		m_ppUILayer[3]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[4]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[5]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		m_ppUILayer[6]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[7]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		m_ppUILayer[8]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		m_ppUILayer[9]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		m_ppUILayer[10]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[11]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[12]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[13]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		m_ppUILayer[14]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[15]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		reinterpret_cast<PartyInviteUI*>(m_ppUILayer[16])->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		reinterpret_cast<InvitationCardUI*>(m_ppUILayer[17])->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		m_ppUILayer[18]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_ppUILayer[19]->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight,
			DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		// UIBar Setting
		reinterpret_cast<UIBar*>(m_ppUILayer[3])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 20, 40, 20 + (m_nWndClientWidth / 10) * 3, 60);
		reinterpret_cast<UIBar*>(m_ppUILayer[3])->SetColorBrush(D2D1::ColorF::Red, 1.0, 20, 40, 20 + (m_nWndClientWidth / 10) * 3, 60);
		reinterpret_cast<UIBar*>(m_ppUILayer[4])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 20, 60, 20 + (m_nWndClientWidth / 10) * 3, 80);
		reinterpret_cast<UIBar*>(m_ppUILayer[4])->SetColorBrush(D2D1::ColorF::Blue, 1.0, 20, 60, 20 + (m_nWndClientWidth / 10) * 3, 80);
		reinterpret_cast<UIBar*>(m_ppUILayer[6])->SetBehindBrush(D2D1::ColorF::Black, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[6])->SetColorBrush(D2D1::ColorF::Red, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[8])->SetBehindBrush(D2D1::ColorF::Black, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70 * 4, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[8])->SetColorBrush(D2D1::ColorF::Red, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70 * 4, (m_nWndClientHeight / 6) - 20);

		reinterpret_cast<UIBar*>(m_ppUILayer[10])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2) - 60, 140, (m_nWndClientHeight / 2) - 40);
		reinterpret_cast<UIBar*>(m_ppUILayer[10])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2) - 60, 140, (m_nWndClientHeight / 2) - 40);
		reinterpret_cast<UIBar*>(m_ppUILayer[11])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2) - 30, 140, (m_nWndClientHeight / 2) - 10);
		reinterpret_cast<UIBar*>(m_ppUILayer[11])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2) - 30, 140, (m_nWndClientHeight / 2) - 10);
		reinterpret_cast<UIBar*>(m_ppUILayer[12])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2), 140, (m_nWndClientHeight / 2) + 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[12])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2), 140, (m_nWndClientHeight / 2) + 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[13])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2) + 30, 140, (m_nWndClientHeight / 2) + 50);
		reinterpret_cast<UIBar*>(m_ppUILayer[13])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2) + 30, 140, (m_nWndClientHeight / 2) + 50);
	}

	Create_OpenWorld_Object();
}

void CGameFramework::Create_OpenWorld_Object()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

#ifdef _WITH_TERRAIN_PLAYER
	CTerrainPlayer* pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain);
	//get_basic_information(pPlayer, 0);

#else
	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif

	m_pScene->m_pPlayer = m_pPlayer = pPlayer;
	m_pCamera = m_pPlayer->GetCamera();
	m_pPlayer->SetUse(true);

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::Create_InDungeon_Object()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

#ifdef _WITH_TERRAIN_PLAYER
	CTerrainPlayer* pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain);
	//get_basic_information(pPlayer, 0);

#else
	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif

	m_pScene->m_pPlayer = m_pPlayer = pPlayer;
	m_pCamera = m_pPlayer->GetCamera();
	m_pPlayer->SetUse(true);

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::Release_OpenWorld_Object()
{
	if (m_pPlayer) m_pPlayer->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::Release_InDungeon_Object()
{
	if (m_pPlayer) m_pPlayer->Release();

	if (m_pRaid_Scene) m_pRaid_Scene->ReleaseObjects();
	if (m_pRaid_Scene) delete m_pRaid_Scene;
}


void CGameFramework::ReleaseObjects()
{
	for (int i = 0; i < UICOUNT; i++) {
		if (!m_ppUILayer[i]) m_ppUILayer[i]->ReleaseResources();
	}
	if (m_ppUILayer)delete m_ppUILayer;

	if (m_pPlayer) m_pPlayer->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;

	if (m_pRaid_Scene) m_pRaid_Scene->ReleaseObjects();
	if (m_pRaid_Scene) delete m_pRaid_Scene;
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		DWORD dwDirection = 0;
		DWORD dwAttack = 0;

		if (!PartyInviteUI_ON && !Chatting_On) {
			if (pKeysBuffer['W'] & 0xF0) {
				dwDirection |= DIR_FORWARD;
			}
			if (pKeysBuffer['S'] & 0xF0) {
				dwDirection |= DIR_BACKWARD;
			}
			if (pKeysBuffer['A'] & 0xF0) {
				dwDirection |= DIR_LEFT;
			}
			if (pKeysBuffer['D'] & 0xF0) {
				dwDirection |= DIR_RIGHT;
			}
		}

		//if (pKeysBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		//if (pKeysBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		//if (pKeysBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		//if (pKeysBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
		if (pKeysBuffer[VK_SPACE] & 0xF0) dwAttack |= 0x30;

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f) || (dwAttack != 0))
		{
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			if (dwDirection) m_pPlayer->Move(dwDirection, /*12.25f*/4.5f, true);
			if (dwAttack) {
				// m_pPlayer->Attack(true);
				send_attack_packet(0);
			}
			send_look_packet(m_pPlayer->GetLookVector(), m_pPlayer->GetRightVector());
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());

	set_myPosition(m_pPlayer->GetPosition());
	send_move_packet(m_pPlayer->GetPosition());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	m_pPlayer->Animate(fTimeElapsed);
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{    
	EnterCriticalSection(&IndunCheck_cs);

	m_GameTimer.Tick(60.0f);
	
	get_basic_information(m_pPlayer, my_id);
	m_pPlayer->SetPosition(return_myPosition());

	ProcessInput();

    AnimateObjects();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();
	
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	// UIRender
	Send_str = L"";
	Hp_str = L"";
	Mp_str = L"";

	// 서버 연결 X용

	string temp_str;
	wstring* party_name_index;
	if (robby_cnt > 0) party_name_index = new wstring[robby_cnt];
	else party_name_index = nullptr;

	// 서버 연결X 용
	/*Info_str = L"";
	Info_str.append(L"Lv : ");
	Info_str.append(L"50");
	Info_str.append(L"  이름 : ");
	Info_str.append(L"홍진선");
	Info_str.append(L"\n직업 : ");
	Info_str.append(L"전사");
	Info_str.append(L"  속성 : ");
	Info_str.append(L"무");*/

	EnterCriticalSection(&UI_cs);
	for (int i = 0; i < UICOUNT; i++) {
		switch (i) {
		case 0: {

			for (auto& m : g_msg) {
				wchar_t* temp;
				//wstring temp = wstring(m.begin(), m.end());
				const char* all = m.c_str();
				int len = 1 + strlen(all);
				temp = new TCHAR[len];
				mbstowcs(temp, all, len);
				Send_str.append(temp);
				Send_str += L"\n";
				delete temp;
			}
			m_ppUILayer[i]->UpdateLabels(Send_str, 0, 340, m_nWndClientWidth / 2, 300 + (m_nWndClientHeight / 3));
		}
			  break;
		case 1:
			m_ppUILayer[i]->UpdateLabels(Chatting_Str, 0, 300 + (m_nWndClientHeight / 3), m_nWndClientWidth / 2, 300 + (m_nWndClientHeight / 3) + 20);
			break;

		case 2:
			m_ppUILayer[i]->UpdateLabels(Info_str, 0, 0, (m_nWndClientWidth / 10) * 3 + 30, 90);
			break;
		case 3: {
			Hp_str.append(L" Hp : ");
			Hp_str.append(to_wstring(m_pPlayer->m_hp));
			Hp_str.append(L" / ");
			Hp_str.append(to_wstring(m_pPlayer->m_max_hp));
			m_ppUILayer[i]->UpdateLabels(Hp_str, 20, 40, 20 + ((float)m_pPlayer->m_hp / m_pPlayer->m_max_hp) * (m_nWndClientWidth / 10) * 3, 60);
			break;
		}
		case 4: {
			Mp_str.append(L" Mp : ");
			Mp_str.append(to_wstring(m_pPlayer->m_mp));
			Mp_str.append(L" / ");
			Mp_str.append(to_wstring(m_pPlayer->m_max_mp));
			m_ppUILayer[i]->UpdateLabels(Mp_str, 20, 60, 20 + ((float)m_pPlayer->m_mp / m_pPlayer->m_max_mp) * (m_nWndClientWidth / 10) * 3, 80);
			break;
		}
		case 5: {
			if (!Combat_On) break;
			m_ppUILayer[i]->UpdateLabels(Combat_str, (m_nWndClientWidth / 2) - 80, 0, (m_nWndClientWidth / 2) + 80, (m_nWndClientHeight / 6));
			break;
		}
		case 6: {
			if (!Combat_On) break;
			wstring ang = L"HP : ";
			ang.append(to_wstring((int)get_combat_id_hp()));
			ang.append(L"/");
			ang.append(to_wstring((int)get_combat_id_max_hp()));
			m_ppUILayer[i]->UpdateLabels(ang, (m_nWndClientWidth / 2) - 70, 40, 140 * ((get_combat_id_hp() / get_combat_id_max_hp())) + ((m_nWndClientWidth / 2) - 70), (m_nWndClientHeight / 6) - 20);
			break;
		}
		case 7: {
			if (!InDungeon) break;

			m_ppUILayer[i]->UpdateLabels(L"가이아", (m_nWndClientWidth / 2) - 80, 0, (m_nWndClientWidth / 2) + 80 * 4, (m_nWndClientHeight / 6));
			break;
		}
		case 8: {
			if (!InDungeon) break;
			int hp_x = (int)get_combat_id_hp() / 100000;
			float bar_percent = (((int)get_combat_id_hp() % 100000) + 1) / 100000.0f;
			wstring ang = L"HP : ";
			ang.append(to_wstring((int)get_combat_id_hp()));
			ang.append(L"/");
			ang.append(to_wstring((int)get_combat_id_max_hp()));
			ang.append(L"\t\t\t\tX");
			ang.append(to_wstring(hp_x));

			// 피(hp_x)에 따른 숫자 바꾸기
			D2D1::ColorF::Enum bc = D2D1::ColorF::Black;
			D2D1::ColorF::Enum fc = D2D1::ColorF::Red;
			switch (hp_x % 5) {
			case 0:
				bc = D2D1::ColorF::Purple;
				fc = D2D1::ColorF::Red;
				break;
			case 1:
				bc = D2D1::ColorF::Red;
				fc = D2D1::ColorF::Orange;
				break;
			case 2:
				bc = D2D1::ColorF::Orange;
				fc = D2D1::ColorF::Green;
				break;
			case 3:
				bc = D2D1::ColorF::Green;
				fc = D2D1::ColorF::DeepSkyBlue;
				break;
			case 4:
				bc = D2D1::ColorF::DeepSkyBlue;
				fc = D2D1::ColorF::Purple;
				break;
			}

			if (hp_x == 0)  bc = D2D1::ColorF::Black;

			reinterpret_cast<UIBar*>(m_ppUILayer[i])->SetBehindBrush(bc, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70 * 4, (m_nWndClientHeight / 6) - 20);
			reinterpret_cast<UIBar*>(m_ppUILayer[i])->SetColorBrush(fc, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70 * 4, (m_nWndClientHeight / 6) - 20);

			m_ppUILayer[i]->UpdateLabels(ang, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) - 70 + (70 * 5) * bar_percent, (m_nWndClientHeight / 6) - 20);
			break;
		}

		case 9: {
			if (!InDungeon) break;
			wstring party_info_str = L"파티원정보(DC : ";
			party_info_str.append(to_wstring(indun_death_count));
			party_info_str.append(L")");
			m_ppUILayer[i]->UpdateLabels(party_info_str, 0, (m_nWndClientHeight / 2) - 80, 150, (m_nWndClientHeight / 2) + 60);
			break;
		}
		case 10: {
			if (!InDungeon) break;
			m_ppUILayer[i]->UpdateLabels(party_name[0], 10, (m_nWndClientHeight / 2) - 60, 10 + 130 * ((float)get_hp_to_server(party_id[0]) / get_max_hp_to_server(party_id[0])), (m_nWndClientHeight / 2) - 40);
			break;
		}
		case 11: {
			if (!InDungeon) break;
			m_ppUILayer[i]->UpdateLabels(party_name[1], 10, (m_nWndClientHeight / 2) - 30, 10 + 130 * ((float)get_hp_to_server(party_id[1]) / get_max_hp_to_server(party_id[1])), (m_nWndClientHeight / 2) - 10);
			break;
		}
		case 12: {
			if (!InDungeon) break;
			m_ppUILayer[i]->UpdateLabels(party_name[2], 10, (m_nWndClientHeight / 2), 10 + 130 * ((float)get_hp_to_server(party_id[2]) / get_max_hp_to_server(party_id[2])), (m_nWndClientHeight / 2) + 20);
			break;
		}
		case 13: {
			if (!InDungeon) break;
			m_ppUILayer[i]->UpdateLabels(party_name[3], 10, (m_nWndClientHeight / 2) + 30, 10 + 130 * ((float)get_hp_to_server(party_id[3]) / get_max_hp_to_server(party_id[3])), (m_nWndClientHeight / 2) + 50);
			break;
		}

		case 15: {
			if (!PartyUI_On) break;
			if (!party_info_on) {
				reinterpret_cast<PartyUI*>(m_ppUILayer[i])->ResizeTextBlock(robby_cnt + 4);
				if (party_id_index_vector.size() != 0) {
					int tmp = 0;
					for (auto t : party_id_index_vector) {
						party_name_index[tmp] = L"NO. ";
						party_name_index[tmp].append(to_wstring(m_party[t]->get_party_id()));
						party_name_index[tmp].append(L"\n 방제 : ");

						// 방 제목 넣기
						wchar_t* temp;
						int len = 1 + strlen(m_party[t]->get_room_name());
						temp = new TCHAR[len];
						mbstowcs(temp, m_party[t]->get_room_name(), len);
						party_name_index[tmp].append(temp);
						delete temp;
						tmp++;
					}
				}
				reinterpret_cast<PartyUI*>(m_ppUILayer[i])->UpdateLabels(party_name_index);
			}
			else {
				reinterpret_cast<PartyUI*>(m_ppUILayer[i])->ResizeTextBlock(robby_cnt + 4 + GAIA_ROOM);
				if (party_id_index_vector.size() != 0) {
					int tmp = 0;
					for (auto t : party_id_index_vector) {
						party_name_index[tmp] = L"NO. ";
						party_name_index[tmp].append(to_wstring(m_party[t]->get_party_id()));
						party_name_index[tmp].append(L"\n 방제 : ");

						// 방 제목 넣기
						wchar_t* temp;
						int len = 1 + strlen(m_party[t]->get_room_name());
						temp = new TCHAR[len];
						mbstowcs(temp, m_party[t]->get_room_name(), len);
						party_name_index[tmp].append(temp);
						delete[]temp;
						tmp++;
					}
				}

				// 파티에 대한 정보 출력
				reinterpret_cast<PartyUI*>(m_ppUILayer[i])->UpdateLabels_PartyInfo(party_name_index, m_party_info, party_enter);
			}

			break;
		}
		case 16: {
			if (!PartyInviteUI_ON) break;
			reinterpret_cast<PartyInviteUI*>(m_ppUILayer[i])->UpdateLabels(Invite_Str);
			break;
		}
		case 17: {
			if (!InvitationCardUI_On) break;
			wstring temp;
			wchar_t* temp2 = get_user_name_to_server(InvitationUser);
			temp.append(temp2);
			temp.append(L"가 ");
			temp.append(std::to_wstring(InvitationRoomId));
			temp.append(L"번 방에 초대하였습니다");
			reinterpret_cast<InvitationCardUI*>(m_ppUILayer[i])->UpdateLabels(temp);
			delete[]temp2;
			break;
		}
		case 18: {
			if (!AddAIUI_On) break;
			reinterpret_cast<AddAIUI*>(m_ppUILayer[i])->UpdateLabels();
			break;
		}
		case 19:
			if (!NoticeUI_On) break;
			m_ppUILayer[i]->UpdateLabels(Notice_str, 0, 0, 640, 50);
			break;
		}
	}

	for (int i = 0; i < UICOUNT; i++) {
		if ((i == 5 || i == 6)) {
			if (!Combat_On) continue;
		}
		if (i >= 7 && i <= 13) {
			if (!InDungeon) continue;
}
		if (i == 15 && !PartyUI_On) {
			continue;
		}
		if (i == 16 && !PartyInviteUI_ON) continue;
		if (i == 17 && !InvitationCardUI_On) continue;
		if (i == 18 && !AddAIUI_On) continue;
		if (i == 19 && !NoticeUI_On) continue;
		m_ppUILayer[i]->Render(m_nSwapChainBufferIndex);
	}
	LeaveCriticalSection(&UI_cs);

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 5/*12*/, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);

	if (robby_cnt > 0) delete[]party_name_index;
	if (InvitationCardUI_On) {
		if (chrono::system_clock::now() > InvitationCardTimer) {
			InvitationCardUI_On = false;
			// 초대 거절 패킷 보내기
		}
	}

	if (NoticeUI_On) {
		if (NoticeTimer < chrono::system_clock::now()) {
			NoticeUI_On = false;
			RaidEnterNotice = false;
			DeadNotice = false;
		}
		else {
			auto t = NoticeTimer - chrono::system_clock::now();
			if (DeadNotice) {
				Notice_str = L"사망했습니다. ";
				Notice_str.append(to_wstring(chrono::duration_cast<chrono::seconds>(t).count()));
				Notice_str.append(L"초 후 부활합니다");
			}

			if (RaidEnterNotice) {
				if (t >= 4s && t < 5s) Notice_str = L"4초후에 게임을 시작합니다";
				else if (t >= 3s && t < 4s) Notice_str = L"3초후에 게임을 시작합니다";
				else if (t >= 2s && t < 3s) Notice_str = L"2초후에 게임을 시작합니다";
				else if (t >= 1s && t < 2s) Notice_str = L"1초후에 게임을 시작합니다";
			}
		}
	}

	LeaveCriticalSection(&IndunCheck_cs);
}

