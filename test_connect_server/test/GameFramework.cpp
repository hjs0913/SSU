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
wstring Send_str = L"";
wstring Hp_str = L"";
wstring Mp_str = L"";

bool Chatting_On = false;
bool Mouse_On = false;


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

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pRaid_Scene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
	CreateDepthStencilView();

	BuildObjects();

	return(true);
}

//#define _WITH_SWAPCHAIN

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
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiSwapChain);
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
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif

	if (!m_pdxgiSwapChain)
	{
		MessageBox(NULL, L"Swap Chain Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}

	if (!m_pd3dDevice)
	{
		hResult = m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	if (!m_pd3dDevice)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
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
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
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

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	//	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);
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
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}




bool CGameFramework::TestIntersection(int mouseX, int mouseY, CGameObject* obj)
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
	origin = m_pCamera->GetPosition();

	// 세계 행렬을 가져와 구의 위치로 변환합니다.  //.여기 다시 보자 
	F3_worldMatrix = m_pCamera->GetLookAtPosition();
	//worldMatrix = XMLoadFloat3(&F3_worldMatrix);
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
	//m_pPlayer
	if (RaySphereIntersect(rayOrigin, rayDirection, 40.0f) == true)
	{
		cout << obj->GetPosition().x << "  " <<obj->GetPosition().y  << "  " << obj->GetPosition().z <<  endl;
		cout << "픽킹";
		return true;
		// 교차하는 경우 화면에 표시되는 텍스트 문자열에서 교차로를 "yes"로 설정합니다.
	//	m_Text->SetIntersection(true, m_D3D->GetDeviceContext());
	}
	else
	{
		return false;
		// 그렇지 않으면 "No"로 교차를 설정하십시오.
	//	m_Text->SetIntersection(false, m_D3D->GetDeviceContext());
	}
}



bool CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (InDungeon) {
		if (m_pRaid_Scene) m_pRaid_Scene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	}
	else {
		if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	}

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN: //좌클릭

	
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

	

	
		if (f4_picking_possible) {
			for (int i = 9615; i < 10615; i++) {  //9615  for (int i = 10615; i < 10795; i++) 
				if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, m_ppObjects[i])) {
					send_picking_skill_packet(2, 0, i);
					m_ppObjects[i]->SetMesh(0, pOtherPlayerMesh[1]);  //피킹 확인위해 색상변경 
					f4_picking_possible = false;
					cout << i << endl;
				}
			}
		}

		if (f5_picking_possible) {
			for (int i = 9615; i < 10615; i++) {  //9615  for (int i = 10615; i < 10795; i++) 
				if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, m_ppObjects[i])) {
					send_picking_skill_packet(0, 0, i);
					m_ppObjects[i]->SetMesh(0, pOtherPlayerMesh[1]);
					f5_picking_possible = false;
					cout << i << endl;
				}
			}
		}

		if (f6_picking_possible) {
			for (int i = 9615; i < 10615; i++) {  //9615  for (int i = 10615; i < 10795; i++) 
				if (TestIntersection(m_ptOldCursorPos.x, m_ptOldCursorPos.y, m_ppObjects[i])) {
					send_picking_skill_packet(1, 0, i);
					m_ppObjects[i]->SetMesh(0, pOtherPlayerMesh[1]);
					f6_picking_possible = false;
					cout << i << endl;
				}
			}
		}
		break;

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
	return false;


}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	char* send_str;
	const wchar_t* temp;
	int len = 0;

	if (InDungeon) {
		if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	}
	else {
		if (m_pRaid_Scene) m_pRaid_Scene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	}

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			Chatting_On = !Chatting_On;
			if (Chatting_On == false) {
				len = 1 + Chatting_Str.length();
				send_str = new char[len*4];
				temp = Chatting_Str.c_str();
				wcstombs(send_str, temp, MAX_CHAT_SIZE);
				send_chat_packet(send_str);
				Chatting_Str=L"";
				delete send_str;
			}
			break;
		case VK_F1:
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
		case VK_F2:
			switch (my_element)
			{
			case E_NONE:
				my_element = E_WATER;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i) 
					m_ppObjects[i]->SetMaterial(pMaterials[8]);
				break;
			case E_WATER:
				my_element = E_FULLMETAL;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[9]);
	
				break;
			case E_FULLMETAL:
				my_element = E_WIND;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[10]);
				break;
			case E_WIND:
				my_element = E_FIRE;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[11]);
				break;
			case E_FIRE:
				my_element = E_TREE;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[12]);
				break;
			case E_TREE:
				my_element = E_EARTH;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[13]);
				break;
			case E_EARTH:
				my_element = E_ICE;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[5]);
				break;
			case E_ICE:
				my_element = E_NONE;
				for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i)
					m_ppObjects[i]->SetMaterial(pMaterials[7]);
				break;
			default:
				break;
			}
			send_change_element_packet(my_element);
			break;
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
		case VK_F10:
			break;
		case 0x50:	// p key
			send_gaia_join_packet();
			break;
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

#if defined(_DEBUG)
	if (m_pd3dDebugController) m_pd3dDebugController->Release();
#endif

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

	netclose();
}

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

		// UIBar Setting
		reinterpret_cast<UIBar*>(m_ppUILayer[3])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 20, 40, 20 + (m_nWndClientWidth / 10) * 3, 60);
		reinterpret_cast<UIBar*>(m_ppUILayer[3])->SetColorBrush(D2D1::ColorF::Red, 1.0, 20, 40, 20 + (m_nWndClientWidth / 10) * 3, 60);
		reinterpret_cast<UIBar*>(m_ppUILayer[4])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 20, 60, 20 + (m_nWndClientWidth / 10) * 3, 80);
		reinterpret_cast<UIBar*>(m_ppUILayer[4])->SetColorBrush(D2D1::ColorF::Blue, 1.0, 20, 60, 20 + (m_nWndClientWidth / 10) * 3, 80);
		reinterpret_cast<UIBar*>(m_ppUILayer[6])->SetBehindBrush(D2D1::ColorF::Black, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[6])->SetColorBrush(D2D1::ColorF::Red, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[8])->SetBehindBrush(D2D1::ColorF::Black, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70*4, (m_nWndClientHeight / 6) - 20);
		reinterpret_cast<UIBar*>(m_ppUILayer[8])->SetColorBrush(D2D1::ColorF::Red, 1.0, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) + 70*4, (m_nWndClientHeight / 6) - 20);
		
		reinterpret_cast<UIBar*>(m_ppUILayer[10])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2) - 60, 140, (m_nWndClientHeight / 2) - 40);
		reinterpret_cast<UIBar*>(m_ppUILayer[10])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2) - 60, 140, (m_nWndClientHeight / 2) - 40);
		reinterpret_cast<UIBar*>(m_ppUILayer[11])->SetBehindBrush(D2D1::ColorF::Black, 1.0, 10, (m_nWndClientHeight / 2) - 30, 140, (m_nWndClientHeight / 2) - 10);
		reinterpret_cast<UIBar*>(m_ppUILayer[11])->SetColorBrush(D2D1::ColorF::Red, 1.0, 10, (m_nWndClientHeight / 2) - 30, 140, (m_nWndClientHeight / 2) - 10);

	}


	Create_OpenWorld_Object();
}

void CGameFramework::Create_OpenWorld_Object()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();
	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pd3dCommandQueue, m_ppd3dSwapChainBackBuffers);


	m_pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->GetTerrain(), 1);
	m_pCamera = m_pPlayer->GetCamera();
	m_pPlayer->SetUse(true);

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	m_GameTimer.Reset();
}

void CGameFramework::Create_InDungeon_Object()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pRaid_Scene = new CScene();
	m_pRaid_Scene->BuildObjects_Raid(m_pd3dDevice, m_pd3dCommandList, m_pd3dCommandQueue, m_ppd3dSwapChainBackBuffers);

	m_pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pRaid_Scene->GetGraphicsRootSignature(), m_pRaid_Scene->GetTerrain(), 1);
	m_pCamera = m_pPlayer->GetCamera();
	m_pPlayer->SetUse(true);

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pRaid_Scene) m_pRaid_Scene->ReleaseUploadBuffers();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	for (int i = 0; i < UICOUNT; i++) {
		if(!m_ppUILayer[i]) m_ppUILayer[i]->ReleaseResources();
	}
	if(m_ppUILayer)delete m_ppUILayer;

	if (m_pPlayer) delete m_pPlayer;

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;

	if (m_pRaid_Scene) m_pRaid_Scene->ReleaseObjects();
	if (m_pRaid_Scene) delete m_pRaid_Scene;
}

void CGameFramework::Release_OpenWorld_Object()
{
	if (m_pPlayer) delete m_pPlayer;
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::Release_InDungeon_Object()
{
	if (m_pPlayer) delete m_pPlayer;
	if (m_pRaid_Scene) m_pRaid_Scene->ReleaseObjects();
	if (m_pRaid_Scene) delete m_pRaid_Scene;
}

void CGameFramework::ProcessInput()   
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	
	if (!InDungeon) {
		if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	}
	else {
		if (GetKeyboardState(pKeysBuffer) && m_pRaid_Scene) bProcessedByScene = m_pRaid_Scene->ProcessInput(pKeysBuffer);
	}
	
	if (!bProcessedByScene)
	{


		static bool pushCTRL = true;

		DWORD dwDirection = 0;

		if (!Chatting_On && Mouse_On) {
			if (pKeysBuffer['W'] & 0xF0) {
				//send_move_packet(0);
				dwDirection |= DIR_FORWARD;
			}
			if (pKeysBuffer['S'] & 0xF0) {
				//send_move_packet(1);
				dwDirection |= DIR_BACKWARD;
			}
			if (pKeysBuffer['A'] & 0xF0) {
				//send_move_packet(2);
				dwDirection |= DIR_LEFT;
			}
			if (pKeysBuffer['D'] & 0xF0) {
				//send_move_packet(3);
				dwDirection |= DIR_RIGHT;
			}
		//	if ((pKeysBuffer[VK_NUMPAD0] & 0xF0) || (pKeysBuffer['0'] & 0xF0)) {   
		//		f4_picking_possible = true;
		//	}

			//��ų---------------------------------
			if ((pKeysBuffer[VK_NUMPAD1] & 0xF0) || (pKeysBuffer['1'] & 0xF0)) {     //   1 
				send_skill_packet(0, 0);
			}
			//	if ((pKeysBuffer[VK_NUMPAD2] & 0xF0) || (pKeysBuffer['2'] & 0xF0)) {     //   2 
			//		send_skill_packet(0, 1);
			//	}
			//	if ((pKeysBuffer[VK_NUMPAD3] & 0xF0) || (pKeysBuffer['3'] & 0xF0)) {     //   3	
			//		send_skill_packet(0, 2);
			//	}

			if (pKeysBuffer[VK_NUMPAD4] & 0xF0 || (pKeysBuffer['4'] & 0xF0)) {     //   4 
				send_skill_packet(1, 0);
			}
			if (pKeysBuffer[VK_NUMPAD5] & 0xF0 || (pKeysBuffer['5'] & 0xF0)) {     //   5 
				send_skill_packet(1, 1);
			
					if (pushCTRL && my_job == J_MAGICIAN) {

						if (shoot) {
							++bulletidx;
							pushCTRL = false;
							shoot = false;
						}
					}
					if (bulletidx >= BULLETCNT + 2) {
						bulletidx = 2;
						for (int i = 0; i < BULLETCNT; ++i) {
							if (IsFire[i]) {
								if(!InDungeon) m_pScene->Rotate(2 + i, 0, tmp[i], 0.0f);
								else m_pRaid_Scene->Rotate(2 + i, 0, tmp[i], 0.0f);
							}
							IsFire[i] = false;
						}
					}
					IsFire[bulletidx - 2] = true;
				
			}
			//	if (pKeysBuffer[VK_NUMPAD6] & 0xF0 || (pKeysBuffer['6'] & 0xF0)) {     //   6
			//		send_skill_packet(1, 2);
			//	}

			if (pKeysBuffer[VK_NUMPAD7] & 0xF0 || (pKeysBuffer['7'] & 0xF0)) {    // 7 
				send_skill_packet(2, 0);
			}
			//if (pKeysBuffer[VK_NUMPAD8] & 0xF0 || (pKeysBuffer['8'] & 0xF0)) {    // 8 
			//	send_skill_packet(2, 1);
			//}
			//if (pKeysBuffer[VK_NUMPAD9] & 0xF0 || (pKeysBuffer['9'] & 0xF0)) {    // 9
		//		send_skill_packet(2, 2);
		//	}
			if (pKeysBuffer[VK_SPACE] & 0xF0) {
				send_attack_packet(0);
			}
			//---------------------------------
			else {
				pushCTRL = true;
			}

			static bool pushq = true;

			if (GetAsyncKeyState('Q') & 0x8000) {
				if (pushq) {
					::gbTerrainTessellationWireframe = !::gbTerrainTessellationWireframe;
					pushq = false;
				}
			}
			else {
				pushq = true;
			}

			if (pKeysBuffer[VK_SHIFT] & 0xF0) {
				cameradis = 1.05f;
			}
			else
				cameradis = 1.0f;

		/*	static bool pushCTRL = true;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (pushCTRL) {
					++bulletidx;
					pushCTRL = false;
				}
			}
			else {
				pushCTRL = true;
			}*/
		}
	
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



		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0) {
					//	m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				}
				else {
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
					for (int i = 0; i < BULLETCNT; ++i) {
						if (!IsFire[i]) {
							if(!InDungeon) m_pScene->Rotate(2 + i, 0, cxDelta, 0.0f);
							else m_pRaid_Scene->Rotate(2 + i, 0, cxDelta, 0.0f);
						}
					}
				}
			}
			
			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
			
			send_look_packet(m_pPlayer->GetLookVector(), m_pPlayer->GetRightVector());
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());


	send_move_packet(m_pPlayer->GetPosition());
}

void CGameFramework::AnimateObjects()
{
	if (InDungeon == false) {
		if (m_pScene) m_pScene->AnimateObjects(m_GameTimer, m_pCamera, m_pPlayer, bulletidx);
	}
	else {
		if (m_pRaid_Scene) m_pRaid_Scene->AnimateObjects(m_GameTimer, m_pCamera, m_pPlayer, bulletidx);
	}
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
	//m_nSwapChainBufferIndex = (m_nSwapChainBufferIndex + 1) % m_nSwapChainBuffers;

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
	m_GameTimer.Tick(0.0f);

	get_basic_information(m_pPlayer, my_id);
	m_pPlayer->SetPosition(return_myPosition());
	ProcessInput();

	// receive Player position to server
	// m_pCamera->Move(return_myCamera());
	//------------------------------------

	
	POINT ptCursorPos;
	if (GetCapture() == m_hWnd)
	{
		SetCursor(NULL);
		GetCursorPos(&ptCursorPos);
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	m_mouseX = m_ptOldCursorPos.x;
	m_mouseY = m_ptOldCursorPos.y;



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
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);


	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (!InDungeon) m_pScene->Render(m_pd3dCommandList, m_pCamera);
	else {
		if (m_pRaid_Scene) m_pRaid_Scene->Render(m_pd3dCommandList, m_pCamera);
		else return;
	}
	XMFLOAT3 tmp = m_pPlayer->GetPosition();
	XMFLOAT3 tmp2 = m_pCamera->GetPosition();


	m_pCamera->SetPosition(XMFLOAT3(
		(tmp2.x - tmp.x) * cameradis + tmp.x,
		(tmp2.y - tmp.y) * cameradis + tmp.y,
		(tmp2.z - tmp.z) * cameradis + tmp.z));


#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();


	// UIRender
	Send_str = L"";
	Hp_str = L"";
	Mp_str = L"";
	string temp_str;
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
			m_ppUILayer[i]->UpdateLabels(ang, (m_nWndClientWidth / 2) - 70, 40, 140*((get_combat_id_hp()/get_combat_id_max_hp()))+((m_nWndClientWidth / 2) - 70), (m_nWndClientHeight / 6) - 20);
			break;
		}
		case 7: {
			if (!InDungeon) break;

			m_ppUILayer[i]->UpdateLabels(L"가이아", (m_nWndClientWidth / 2) - 80, 0, (m_nWndClientWidth / 2) + 80*4, (m_nWndClientHeight / 6));
			break; 
		}
		case 8: {
			if (!InDungeon) break;
			int hp_x = (int)get_combat_id_hp() / 100000;
			float bar_percent = (((int)get_combat_id_hp() % 100000)+1)/100000.0f;
			wstring ang = L"HP : ";
			ang.append(to_wstring((int)get_combat_id_hp()));
			ang.append(L"/");
			ang.append(to_wstring((int)get_combat_id_max_hp()));
			ang.append(L"\t\t\t\tX");
			ang.append(to_wstring(hp_x));

			// 피(hp_x)에 따른 숫자 바꾸기
			D2D1::ColorF::Enum bc = D2D1::ColorF::Black;
			D2D1::ColorF::Enum fc = D2D1::ColorF::Red;
			switch (hp_x%5) {
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
			
			m_ppUILayer[i]->UpdateLabels(ang, (m_nWndClientWidth / 2) - 70, 40, (m_nWndClientWidth / 2) - 70 + (70*5)*bar_percent, (m_nWndClientHeight / 6) - 20);
			break;
		}
		case 9: {
			if (!InDungeon) break;
			wstring party_info_str = L"파티원정보(DC : ";
			party_info_str.append(to_wstring(indun_death_count));
			party_info_str.append(L")");
			m_ppUILayer[i]->UpdateLabels(party_info_str, 0, (m_nWndClientHeight / 2)-80, 150, (m_nWndClientHeight / 2)+20);
			break; 
		}
		case 10: {
			if (!InDungeon) break;
			get_hp_to_server(party_id[0]);
			m_ppUILayer[i]->UpdateLabels(party_name[0], 10, (m_nWndClientHeight / 2) - 60, 10+130*((float)get_hp_to_server(party_id[0])/ get_max_hp_to_server(party_id[0])), (m_nWndClientHeight / 2) - 40);
			break;
		}
		case 11: {
			if (!InDungeon) break;
			m_ppUILayer[i]->UpdateLabels(party_name[1], 10, (m_nWndClientHeight / 2) - 30, 10 + 130 * ((float)get_hp_to_server(party_id[1]) / get_max_hp_to_server(party_id[1])), (m_nWndClientHeight / 2) - 10);
			break;
		}
	}
	}

	for (int i = 0; i < UICOUNT; i++) {
		if ((i == 5 || i == 6)) {
			if (!Combat_On) continue;
		}
		if (i >= 7 && i <= 11) {
			if (!InDungeon) continue;
		}
		m_ppUILayer[i]->Render(m_nSwapChainBufferIndex);

	}
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

	//	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	MoveToNextFrame();


	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

