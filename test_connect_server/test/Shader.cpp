//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "GameFramework.h"
#include "DDSTextureLoader12.h"
#include "Player.h"
#include <fstream>
#include <iostream>
using namespace std;
#define BULLETCNT 100
#define ROOMX 4000
#define ROOMZ 4000

CHeightMapTerrain* map;
 float hp_width = 50;
 float hp_height = 50;
float start[BULLETCNT];



CShader::CShader()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

CShader::~CShader()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; i++)
			if (m_ppd3dPipelineStates[i])
				m_ppd3dPipelineStates[i]->Release();
		delete[] m_ppd3dPipelineStates;
	}
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateDomainShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateHullShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	::D3DCompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//if (pd3dGraphicsRootSignature)
	//{
	//	m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	//	pd3dGraphicsRootSignature->AddRef();
	//}

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dHullShaderBlob = NULL, * pd3dDomainShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.HS = CreateHullShader(&pd3dHullShaderBlob);
	d3dPipelineStateDesc.DS = CreateDomainShader(&pd3dDomainShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);
	DefaultPipelineState = d3dPipelineStateDesc;
	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	if (pd3dHullShaderBlob) pd3dHullShaderBlob->Release();
	if (pd3dDomainShaderBlob) pd3dDomainShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
}

void CShader::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorStartHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorStartHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorStartHandle);
		m_d3dSrvCPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorStartHandle);
		m_d3dSrvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
}

void CShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShader::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
}

void CShader::ReleaseShaderVariables()
{
	//	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
}

void CShader::ReleaseUploadBuffers()
{
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender(pd3dCommandList, pCamera, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlayerShader::CPlayerShader()
{
}

CPlayerShader::~CPlayerShader()
{
}

D3D12_INPUT_LAYOUT_DESC CPlayerShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CPlayerShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSPlayer", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CPlayerShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSPlayer", "ps_5_1", ppd3dShaderBlob));
}

void CPlayerShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

void CTexturedShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
	}
}

void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CTexturedShader::ReleaseShaderVariables();
}
class Obstacle
{
public:
	int _id;
	float _x;
	float _y;
	float _z;
};

void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList ,void* pContext)
{


	Obstacle obstacles[609];
	ifstream obstacles_read("tree_position.txt");
	if (!obstacles_read.is_open()) {
		cout << "파일을 읽을 수 없습니다" << endl;
		return;
	}
	for (int i = 0; i < 609; i++) {
		float x, y, z;
		obstacles_read >> x >> y >> z;
	//	cout << x << "," << y << "," << z << endl;
		obstacles[i]._id = i;
		obstacles[i]._x = x +2500;
		obstacles[i]._y = y + 200;
		obstacles[i]._z = z + 2500;
	}

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	map = pTerrain;
	float fxPitch = 12.0f * 3.5f;
	float fyPitch = 12.0f * 3.5f;
	float fzPitch = 12.0f * 3.5f;

	float fTerrainWidth = pTerrain->GetWidth();
	float fTerrainLength = pTerrain->GetLength();

	int xObjects = int(fTerrainWidth / fxPitch);   //97
	int yObjects = 1;
	int zObjects = int(fTerrainLength / fzPitch);
	m_nObjects = (xObjects * yObjects * zObjects);  //97

	m_nObjects += 1 + 2 * BULLETCNT + 1 + 4;
#define TEXTURES 7
	CTexture* pTexture[TEXTURES];
	pTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/hp.dds", RESOURCE_TEXTURE2D, 0);   //여기 

	pTexture[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tree02.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mp.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[3] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/house.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[4] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/roof.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[5] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/flare.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mirror.dds", RESOURCE_TEXTURE2D, 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 7);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	for (int i = 0; i < TEXTURES; i++) CreateShaderResourceViews(pd3dDevice, pTexture[i], 0, 3);

#ifdef _WITH_BATCH_MATERIAL
	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
#else
	//CMaterial* pMaterials[TEXTURES];
	for (int i = 0; i < TEXTURES; i++)
	{
		pMaterials[i] = new CMaterial();
		pMaterials[i]->SetTexture(pTexture[i]);
	}
#endif
	for (int i = 0; i < 100; ++i) {
		newhp[i] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, i, hp_height, 0.0f);
		newmp[i] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, i, hp_height, 0.0f);
	}

	pRectMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, hp_width, hp_height, 0.0f);
	
	CTexturedRectMesh* part = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5, 5, 0.0f);

	CReverseCubeMeshTextured* room = new CReverseCubeMeshTextured(pd3dDevice, pd3dCommandList, 300.0f, 100.0f, 300.0f);

	CCubeMeshDiffused* bullet = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 1.0f);

	Car* car = new Car(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);

	m_ppObjects = new CGameObject * [m_nObjects];
	//CBillboardObject* pBillboardObject = NULL;

	CGameObject* phouseObject = new CGameObject(1);

	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, room);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[3]);
#endif
	phouseObject->SetPosition(ROOMX, 50, ROOMZ);

	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	m_ppObjects[0] = phouseObject;

	CBulletObject* bulletmesh;
	for (int i = 1; i < 1 + BULLETCNT; ++i) {
		bulletmesh = new CBulletObject(1);
		bulletmesh->SetMesh(0, bullet);
		//bulletmesh->SetMaterial(pMaterials[(i - 2) % 2 + 3]);

		bulletmesh->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = bulletmesh;
	}

	for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i) {
		pBillboardObject = new CBillboardObject(1);
		pBillboardObject->SetMesh(0, part);
#ifndef _WITH_BATCH_MATERIAL
		pBillboardObject->SetMaterial(pMaterials[5]);
#endif
		pBillboardObject->SetPosition(0, -100, 0);
		pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = pBillboardObject;
	}

	std::default_random_engine dre;
	std::uniform_int_distribution<> uid{ 0,2 };

	int tmp;
	int num = 0;
	for (int i = 1 + 2 * BULLETCNT, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				//pBillboardObject = new CBillboardObject(1);
				//pBillboardObject->SetMesh(0, pRectMesh);
#ifndef _WITH_BATCH_MATERIAL    

				if (i == 201) { // hp
					pBillboardObject->SetMesh(0, newhp[50]);
					pBillboardObject->SetMaterial(pMaterials[0]);  //여기
				
				}
				else if (i == 202) {// mp
					pBillboardObject->SetMesh(0, newmp[50]);
					pBillboardObject->SetMaterial(pMaterials[2]);
				
				}
				else {
					pBillboardObject->SetMesh(0, pRectMesh);
					pBillboardObject->SetMaterial(pMaterials[1]);
					//	pBillboardObject->SetMaterial(pMaterials[uid(dre)]);
				}
		
#endif
				// 장애물 인덱스 생각(기윤)
				float xPosition = obstacles[x  + z]._x;
				float zPosition = obstacles[x  + z ]._z;
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				//cout << xPosition << " " << fHeight << " " << zPosition << endl;
			//	if (xPosition <= fTerrainWidth / 2 - 200 || xPosition >= fTerrainWidth / 2 + 200 ||   //나무 위치     
				//	zPosition <= fTerrainLength / 2 - 200 || zPosition >= fTerrainLength / 2 + 200) {
				pBillboardObject->SetPosition(xPosition, 35, zPosition);         //1028 168 1028
				//cout << hp_pos.x << hp_pos.y << hp_pos.z << endl;

		//	}
			//if (x == 1)
			//pBillboardObject->SetPosition(xPosition, fHeight, zPosition);
				pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
				m_ppObjects[i++] = pBillboardObject;
				tmp = i;

			}
			num += 1;
			if (num > 9)
				num = 9;
		}
	
	}


	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, car);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	phouseObject->SetPosition(ROOMX + 30, 12, ROOMZ - 10);

	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * tmp);
	m_ppObjects[tmp] = phouseObject;





	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, room);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[3]);
#endif
	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);
	phouseObject->SetPosition(ROOMX, 50, ROOMZ + 300);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, car);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 00);
	R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);
	phouseObject->SetPosition(ROOMX + 30, 12, ROOMZ + 310);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	Skull* skull = new Skull(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 2.0f);
	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, skull);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[5]);
#endif
	mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0); // xy plane
	R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);
	
//	phouseObject->SetPosition(ROOMX, 0, ROOMZ + 300);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	CTexturedRectMesh* mirror = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 100, 100, 0.0f,0,0,1);
	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, mirror);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	phouseObject->SetPosition(ROOMX, 50, ROOMZ + 150);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;
}

void CObjectsShader::BuildObjects2(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext, ID3D12RootSignature* pd3dGraphicsRootSignature)
{


	Obstacle obstacles[609];
	ifstream obstacles_read("tree_position.txt");
	if (!obstacles_read.is_open()) {
		cout << "파일을 읽을 수 없습니다" << endl;
		return;
	}
	for (int i = 0; i < 609; i++) {
		float x, y, z;
		obstacles_read >> x >> y >> z;
		//	cout << x << "," << y << "," << z << endl;
		obstacles[i]._id = i;
		obstacles[i]._x = x;
		obstacles[i]._y = y;
		obstacles[i]._z = z;
	}

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	map = pTerrain;
	float fxPitch = 12.0f * 3.5f;
	float fyPitch = 12.0f * 3.5f;
	float fzPitch = 12.0f * 3.5f;

	float fTerrainWidth = pTerrain->GetWidth();
	float fTerrainLength = pTerrain->GetLength();

	int xObjects = int(fTerrainWidth / fxPitch);   //97
	int yObjects = 1;
	int zObjects = int(fTerrainLength / fzPitch);
	m_nObjects = (xObjects * yObjects * zObjects);  //97

	// m_nObjects += 1 + 2 * BULLETCNT + 1 + 4;
	m_nObjects += 1 + 2 * BULLETCNT + 1 + 4 + MAX_NPC + MAX_USER;
#define TEXTURES 7
	CTexture* pTexture[TEXTURES];
	pTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/hp.dds", RESOURCE_TEXTURE2D, 0);   //여기 

	pTexture[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tree02.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mp.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[3] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/house.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[4] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/roof.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[5] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/flare.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mirror.dds", RESOURCE_TEXTURE2D, 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 7);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	for (int i = 0; i < TEXTURES; i++) CreateShaderResourceViews(pd3dDevice, pTexture[i], 0, 3);

#ifdef _WITH_BATCH_MATERIAL
	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
#else
	//CMaterial* pMaterials[TEXTURES];
	for (int i = 0; i < TEXTURES; i++)
	{
		pMaterials[i] = new CMaterial();
		pMaterials[i]->SetTexture(pTexture[i]);
	}
#endif

	for (int i = 0; i < 100; ++i) {
		newhp[i] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, i, hp_height, 0.0f);
		newmp[i] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, i, hp_height, 0.0f);
	}
	pRectMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, hp_width, hp_height, 0.0f);


	CTexturedRectMesh* part = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5, 5, 0.0f);

	CReverseCubeMeshTextured* room = new CReverseCubeMeshTextured(pd3dDevice, pd3dCommandList, 300.0f, 100.0f, 300.0f);

	CCubeMeshDiffused* bullet = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 1.0f);

	Car* car = new Car(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);

	pOtherPlayerMesh[0] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList, 
		20.0f, 20.0f, 4.0f, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	pOtherPlayerMesh[1] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	pOtherPlayerMesh[2] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	pOtherPlayerMesh[3] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
	pOtherPlayerMesh[4] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
	pOtherPlayerMesh[5] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
	pOtherPlayerMesh[6] = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList,
		20.0f, 20.0f, 4.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	//CAirplaneMeshDiffused* npc = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList);

	m_ppObjects = new CGameObject * [m_nObjects];
	//CBillboardObject* pBillboardObject = NULL;

	CGameObject* phouseObject = new CGameObject(1);

	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, room);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[3]);
#endif
	phouseObject->SetPosition(ROOMX, 50, ROOMZ);

	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	m_ppObjects[0] = phouseObject;

	CBulletObject* bulletmesh;
	for (int i = 1; i < 1 + BULLETCNT; ++i) {
		bulletmesh = new CBulletObject(1);
		bulletmesh->SetMesh(0, bullet);
		//bulletmesh->SetMaterial(pMaterials[(i - 2) % 2 + 3]);

		bulletmesh->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = bulletmesh;
	}

	for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i) {
		pBillboardObject = new CBillboardObject(1);
		pBillboardObject->SetMesh(0, part);
#ifndef _WITH_BATCH_MATERIAL
		pBillboardObject->SetMaterial(pMaterials[5]);
#endif
		pBillboardObject->SetPosition(0, -100, 0);
		pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = pBillboardObject;
	}

	std::default_random_engine dre;
	std::uniform_int_distribution<> uid{ 0,2 };

	int tmp;
	int num = 0;
	for (int i = 1 + 2 * BULLETCNT, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				if (i == 201) {
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, newhp[50]);
				}
				else if (i == 202) {
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, newmp[50]);
				}
				else {
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, pRectMesh);
				}
#ifndef _WITH_BATCH_MATERIAL    

				if (i == 201) //HP
					pBillboardObject->SetMaterial(pMaterials[0]);  //여기
				else if (i == 202) { // mp
				
					pBillboardObject->SetMaterial(pMaterials[2]);
			
				}
				else if (i > 202)
					pBillboardObject->SetMaterial(pMaterials[1]);
				//	pBillboardObject->SetMaterial(pMaterials[uid(dre)]);

#endif
				// 장애물 인덱스 생각(기윤)
				float xPosition;
				float zPosition;
				if (x * 97 + z >= 609) {
					xPosition = 0.0f;
					zPosition = 0.0f;
				}
				else {
					xPosition = obstacles[x * 97 + z]._x + 100;
					zPosition = obstacles[x * 97 + z]._z + 300;
				}

				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				//cout << xPosition << " " << fHeight << " " << zPosition << endl;
			//	if (xPosition <= fTerrainWidth / 2 - 200 || xPosition >= fTerrainWidth / 2 + 200 ||   //나무 위치     
				//	zPosition <= fTerrainLength / 2 - 200 || zPosition >= fTerrainLength / 2 + 200) {
				pBillboardObject->SetPosition(xPosition, fHeight+23, zPosition);         //1028 168 1028
				if (pBillboardObject->GetPosition().y >= 50) {
					pBillboardObject->SetPosition(xPosition, 23, zPosition);
				}
				
				pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
				m_ppObjects[i++] = pBillboardObject;
			
				tmp = i;
			}
			num += 1;
			if (num > 9)
				num = 9;
		}

	}


	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, car);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	phouseObject->SetPosition(ROOMX + 30, 12, ROOMZ - 10);

	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * tmp);
	m_ppObjects[tmp] = phouseObject;





	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, room);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[3]);
#endif
	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);
	phouseObject->SetPosition(ROOMX, 50, ROOMZ + 300);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, car);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 00);
	R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);
	phouseObject->SetPosition(ROOMX + 30, 12, ROOMZ + 310);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	Skull* skull = new Skull(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 2.0f);
	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, skull);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[5]);
#endif
	mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0); // xy plane
	R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&phouseObject->m_xmf4x4World, XMLoadFloat4x4(&phouseObject->m_xmf4x4World) * R);

	//	phouseObject->SetPosition(ROOMX, 0, ROOMZ + 300);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;


	CTexturedRectMesh* mirror = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 100, 100, 0.0f, 0, 0, 1);
	phouseObject = new CGameObject(1);
	phouseObject->SetMesh(0, mirror);
#ifndef _WITH_BATCH_MATERIAL
	phouseObject->SetMaterial(pMaterials[6]);
#endif
	phouseObject->SetPosition(ROOMX, 50, ROOMZ + 150);
	phouseObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + ::gnCbvSrvDescriptorIncrementSize * (++tmp));
	m_ppObjects[tmp] = phouseObject;

	// 플레이어
	for (int i = m_nObjects - MAX_NPC - MAX_USER; i < m_nObjects - MAX_NPC; i++) {
		CAirplanePlayer* pOtherPlayer = new CAirplanePlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
		pOtherPlayer->SetMesh(0, pOtherPlayerMesh[2]);
		pOtherPlayer->SetPosition(XMFLOAT3(0, -100, 0));
		pOtherPlayer->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
		m_ppObjects[i] = pOtherPlayer;
	}

	// NPC
	for (int i = m_nObjects - MAX_NPC; i < m_nObjects; i++) {
		CAirplanePlayer* pNpc = new CAirplanePlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
		pNpc->SetMesh(0, pOtherPlayerMesh[1]);
		pNpc->SetPosition(XMFLOAT3(0, -100, 0));
		pNpc->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
		m_ppObjects[i] = pNpc;
	}
}

void CObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) delete m_pMaterial;
#endif
}

void CObjectsShader::AnimateObjects(CGameTimer pTimer, CCamera* pCamera, CGameObject* player, int bulletidx)
{
	CAirplanePlayer* pPlayer = NULL;
	int server_id = MAX_USER + MAX_NPC;
	int MAX_WORLD_SHADER = m_nObjects - server_id;
	for (int j = 0; j < m_nObjects; j++)
	{
		if (j >= m_nObjects - server_id) {
			pPlayer = reinterpret_cast<CAirplanePlayer*>(m_ppObjects[j]);
			bool tp = pPlayer->GetUse();
			pPlayer->SetUse(get_use_to_server(j-MAX_WORLD_SHADER));
			if (pPlayer->GetUse()) {
				if (tp != pPlayer->GetUse()) {
					// 최초 정보 불러오기 및 종족에 맞게 변환
					get_basic_information(pPlayer, j - MAX_WORLD_SHADER);
					if (pPlayer->m_tribe == HUMAN) {
						pPlayer->SetMesh(0, pOtherPlayerMesh[2]);
						get_player_information(pPlayer, j - MAX_WORLD_SHADER);
					}
					else {
						switch (pPlayer->m_spices)
						{
						case FALLEN_FLOG: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[1]);
							break;
						}
						case FALLEN_CHICKEN: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[3]);
							break;
						}
						case FALLEN_RABBIT: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[6]);
							break;
						}
						case FALLEN_MONKEY: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[5]);
							break;
						}
						case WOLF_BOSS: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[4]);
							break;
						}
						case FALLEN_TIGER: {
							pPlayer->SetMesh(0, pOtherPlayerMesh[0]);
							break;
						}
						default:
							break;
						}
					}
				}
				// 이때만 렌더링
				m_ppObjects[j]->SetPosition(get_position_to_server(j - MAX_WORLD_SHADER));
				pPlayer->SetLook(get_look_to_server(j - MAX_WORLD_SHADER));
				pPlayer->Animate(pTimer, pCamera, m_ppObjects[j]);
			}
			else {
				m_ppObjects[j]->SetPosition(XMFLOAT3(0, -100, 0));
				pPlayer->Animate(pTimer, pCamera, m_ppObjects[j]);
			}
		}
		else if (j <= bulletidx) {
			if (m_ppObjects[j]->GetPosition().x < 0 || m_ppObjects[j]->GetPosition().x>5000 ||
				m_ppObjects[j]->GetPosition().z < 0 || m_ppObjects[j]->GetPosition().z>5000) {
				m_ppObjects[j]->SetPosition(0, -100, 0);
			}
			else if (m_ppObjects[j]->Animate(pTimer, pCamera, player, map)) {

				if (start[j] == 0)
					start[j] = pTimer.GetTotalTime();

				m_ppObjects[j + BULLETCNT]->SetPosition(
					(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
					(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
					(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);
					m_ppObjects[j + BULLETCNT]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT]->GetPosition(), 0);

					if (j < 70) {
						m_ppObjects[j + BULLETCNT+30]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);
						m_ppObjects[j + BULLETCNT + 31]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);
						m_ppObjects[j + BULLETCNT + 32]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);

					m_ppObjects[j + BULLETCNT + 32]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT + 32]->GetPosition(), 1);
					m_ppObjects[j + BULLETCNT + 31]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT + 31]->GetPosition(), 2);
					m_ppObjects[j + BULLETCNT + 30]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT + 30]->GetPosition(), 3);
					}
					else {
						m_ppObjects[j + BULLETCNT -30]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);
						m_ppObjects[j + BULLETCNT - 31]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);
						m_ppObjects[j + BULLETCNT - 32]->SetPosition(
							(m_ppObjects[j]->GetPosition().x - player->GetPosition().x) * 0.9 + player->GetPosition().x,
							(m_ppObjects[j]->GetPosition().y - player->GetPosition().y) * 0.9 + player->GetPosition().y,
							(m_ppObjects[j]->GetPosition().z - player->GetPosition().z) * 0.9 + player->GetPosition().z);

						m_ppObjects[j + BULLETCNT - 32]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT - 32]->GetPosition(), 1);
						m_ppObjects[j + BULLETCNT - 31]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT - 31]->GetPosition(), 2);
						m_ppObjects[j + BULLETCNT - 30]->AnimatePart(pTimer, start[j], m_ppObjects[j + BULLETCNT - 30]->GetPosition(), 3);
					}

				if (pTimer.GetTotalTime() - start[j] >= 2) {
					m_ppObjects[j]->SetPosition(0, -100, 0);
					start[j] = 0;
				}
			}
		}
		else if (j == m_nObjects-2) {
			m_ppObjects[j]->SyncPlayer(pTimer, pCamera, player);
		}
		else {
			if (j == 201) {
			//	pRectMesh1 = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, hp_width * 0.5, hp_height, 0.0f);

				m_ppObjects[j]->Animate(pTimer, pCamera, player);
			}
			m_ppObjects[j]->Animate(pTimer, pCamera, player);
		}

	}
}

void CObjectsShader::RotateObject(int i, float x, float y, float z) {
	m_ppObjects[i]->Rotate(x, y, z);
}

void CObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
#endif
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CTexturedShader::Render(pd3dCommandList, pCamera);

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
#endif

	for (int j = 0; j < m_nObjects - 4; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
	pd3dCommandList->OMSetStencilRef(1);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[1]);

	if (m_ppObjects[m_nObjects - 1]) m_ppObjects[m_nObjects - 1]->Render(pd3dCommandList, pCamera);  //거울 스텐실에

	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[2]);

	for (int j = m_nObjects - 4; j < m_nObjects-1; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);   //반사 객체
	}

	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[3]);

	if (m_ppObjects[m_nObjects - 1]) m_ppObjects[m_nObjects - 1]->Render(pd3dCommandList, pCamera);  //거울
}

void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature) {
	if (pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
		pd3dGraphicsRootSignature->AddRef();
	}
	m_nPipelineStates = 4;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	D3D12_DEPTH_STENCIL_DESC mirror;
	mirror.DepthEnable = true;
	mirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirror.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirror.StencilEnable = true;
	mirror.StencilReadMask = 0xff;
	mirror.StencilWriteMask = 0xff;
	mirror.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mirror.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirror.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_BLEND_DESC mirrorBlend;
	mirrorBlend.AlphaToCoverageEnable = false;
	mirrorBlend.IndependentBlendEnable = false;
	mirrorBlend.RenderTarget[0].BlendEnable = false;
	mirrorBlend.RenderTarget[0].LogicOpEnable = false;
	mirrorBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	mirrorBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	mirrorBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	mirrorBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	mirrorBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	mirrorBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	mirrorBlend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	mirrorBlend.RenderTarget[0].RenderTargetWriteMask = 0;

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.DepthStencilState = mirror;
	d3dPipelineStateDesc.BlendState = mirrorBlend;
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;


	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);


	mirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	mirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	mirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = mirror;
	d3dPipelineStateDesc.RasterizerState.FrontCounterClockwise = true;

	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[2]);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = DefaultPipelineState;
	transparentPsoDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	transparentPsoDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	transparentPsoDesc.InputLayout = CreateInputLayout();

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;

	hResult = pd3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[3]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", ppd3dShaderBlob));
}

void CTerrainShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", ppd3dShaderBlob));
}

void CSkyBoxShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}
////////////////////////
D3D12_INPUT_LAYOUT_DESC CTerrainTessellationShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CTerrainTessellationShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrainTessellation", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrainTessellation", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateHullShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "HSTerrainTessellation", "hs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateDomainShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "DSTerrainTessellation", "ds_5_1", ppd3dShaderBlob));
}

void CTerrainTessellationShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{

	if (pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
		pd3dGraphicsRootSignature->AddRef();
	}

	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dHullShaderBlob = NULL, * pd3dDomainShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.HS = CreateHullShader(&pd3dHullShaderBlob);
	d3dPipelineStateDesc.DS = CreateDomainShader(&pd3dDomainShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);

	d3dPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	if (pd3dHullShaderBlob) pd3dHullShaderBlob->Release();
	if (pd3dDomainShaderBlob) pd3dDomainShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CTerrainTessellationShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState((::gbTerrainTessellationWireframe) ? m_ppd3dPipelineStates[1] : m_ppd3dPipelineStates[0]);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);
}
//////////////////////////////
D3D12_INPUT_LAYOUT_DESC MirrorShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE MirrorShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE MirrorShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}
void MirrorShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature) {
	if (pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
		pd3dGraphicsRootSignature->AddRef();
	}
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	D3D12_DEPTH_STENCIL_DESC mirror;
	mirror.DepthEnable = true;
	mirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirror.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirror.StencilEnable = true;
	mirror.StencilReadMask = 0xff;
	mirror.StencilWriteMask = 0xff;
	mirror.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mirror.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirror.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirror.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_BLEND_DESC mirrorBlend;
	mirrorBlend.AlphaToCoverageEnable = false;
	mirrorBlend.IndependentBlendEnable = false;
	mirrorBlend.RenderTarget[0].BlendEnable = false;
	mirrorBlend.RenderTarget[0].LogicOpEnable = false;
	mirrorBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	mirrorBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	mirrorBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	mirrorBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	mirrorBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	mirrorBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	mirrorBlend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	mirrorBlend.RenderTarget[0].RenderTargetWriteMask = 0;

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = mirrorBlend;
	d3dPipelineStateDesc.DepthStencilState = mirror;
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void MirrorShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext) {
	pd3dCommandList->OMSetStencilRef(1);
	//	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
	//	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

}
//////////////////////////////////
D3D12_INPUT_LAYOUT_DESC ReflectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE ReflectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ReflectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob) {
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

void ReflectShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature) {
	if (pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
		pd3dGraphicsRootSignature->AddRef();
	}
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	//D3D12_DEPTH_STENCIL_DESC mirror;
	//mirror.DepthEnable = true;
	//mirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//mirror.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//mirror.StencilEnable = true;
	//mirror.StencilReadMask = 0xff;
	//mirror.StencilWriteMask = 0xff;
	//mirror.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//mirror.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//mirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//mirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	//mirror.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//mirror.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//mirror.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	//mirror.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	//D3D12_BLEND_DESC mirrorBlend;
	//mirrorBlend.AlphaToCoverageEnable = false;
	//mirrorBlend.IndependentBlendEnable = false;
	//mirrorBlend.RenderTarget[0].BlendEnable = false;
	//mirrorBlend.RenderTarget[0].LogicOpEnable = false;
	//mirrorBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	//mirrorBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	//mirrorBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	//mirrorBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	//mirrorBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	//mirrorBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//mirrorBlend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//mirrorBlend.RenderTarget[0].RenderTargetWriteMask = 0;

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;


	//D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	//::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	//d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	//d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	//d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	//d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	//d3dPipelineStateDesc.BlendState = mirrorBlend;
	//d3dPipelineStateDesc.DepthStencilState = mirror;
	//d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	//d3dPipelineStateDesc.SampleMask = UINT_MAX;
	//d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//d3dPipelineStateDesc.NumRenderTargets = 1;
	//d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//d3dPipelineStateDesc.SampleDesc.Count = 1;
	//d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	//d3dPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//d3dPipelineStateDesc.RasterizerState.FrontCounterClockwise = true;



	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void ReflectShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext) {
	//	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
	//	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

}