//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "GameFramework.h"
#include "DDSTextureLoader12.h"
#include "Player.h"
#include <fstream>

#define BULLETCNT 100

CHeightMapTerrain* map;
float hp_width = 50;
float hp_height = 50;
float start[BULLETCNT];

CTexturedRectMesh* newhp[100];
CTexturedRectMesh* newmp[100];
CCubeMeshDiffused* bullet;

ST_SPHERE sphere[180];

CShader::CShader()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; i++)
			if (m_ppd3dPipelineStates[i])
				m_ppd3dPipelineStates[i]->Release();
		delete[] m_ppd3dPipelineStates;
	}

	//if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
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

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char *pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char *)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob)
{
	UINT nReadBytes = 0;
#ifdef _WITH_WFOPEN
	FILE *pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	nReadBytes = (UINT)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
#endif
#ifdef _WITH_STD_STREAM
	std::ifstream ifsFile;
	ifsFile.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	nReadBytes = (int)ifsFile.tellg();
	BYTE *pByteCode = new BYTE[*pnReadBytes];
	ifsFile.seekg(0);
	ifsFile.read((char *)pByteCode, nReadBytes);
	ifsFile.close();
#endif

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	if (ppd3dShaderBlob)
	{
		*ppd3dShaderBlob = NULL;
		HRESULT hResult = D3DCreateBlob(nReadBytes, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode, nReadBytes);
		d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	}
	else
	{
		d3dShaderByteCode.BytecodeLength = nReadBytes;
		d3dShaderByteCode.pShaderBytecode = pByteCode;
	}

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
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
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

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dHullShaderBlob = NULL, * pd3dDomainShaderBlob = NULL;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_d3dPipelineStateDesc.NumRenderTargets = 1;
	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	//HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);
	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState)
{
	//if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);
}


void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
	//OnPrepareRender(pd3dCommandList, pCamera, NULL);
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
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////
//D3D12_INPUT_LAYOUT_DESC CTerrainTessellationShader::CreateInputLayout()
//{
//	UINT nInputElementDescs = 4;
//	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
//
//	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
//	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
//	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
//	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
//
//	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
//	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
//	d3dInputLayoutDesc.NumElements = nInputElementDescs;
//
//	return(d3dInputLayoutDesc);
//}
//
//D3D12_RASTERIZER_DESC CTerrainTessellationShader::CreateRasterizerState()
//{
//	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
//	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
//	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
//	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
//	d3dRasterizerDesc.DepthBias = 0;
//	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
//	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
//	d3dRasterizerDesc.DepthClipEnable = TRUE;
//	d3dRasterizerDesc.MultisampleEnable = FALSE;
//	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
//	d3dRasterizerDesc.ForcedSampleCount = 0;
//	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
//
//	return(d3dRasterizerDesc);
//}
//
//D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrainTessellation", "vs_5_1", ppd3dShaderBlob));
//}
//
//D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrainTessellation", "ps_5_1", ppd3dShaderBlob));
//}
//
//D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateHullShader(ID3DBlob** ppd3dShaderBlob)
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "HSTerrainTessellation", "hs_5_1", ppd3dShaderBlob));
//}
//
//D3D12_SHADER_BYTECODE CTerrainTessellationShader::CreateDomainShader(ID3DBlob** ppd3dShaderBlob)
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "DSTerrainTessellation", "ds_5_1", ppd3dShaderBlob));
//}
//
//void CTerrainTessellationShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
//{
//
//	if (pd3dGraphicsRootSignature)
//	{
//		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
//		pd3dGraphicsRootSignature->AddRef();
//	}
//
//	m_nPipelineStates = 2;
//	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
//
//	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dHullShaderBlob = NULL, * pd3dDomainShaderBlob = NULL;
//
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
//	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
//	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
//	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
//	d3dPipelineStateDesc.HS = CreateHullShader(&pd3dHullShaderBlob);
//	d3dPipelineStateDesc.DS = CreateDomainShader(&pd3dDomainShaderBlob);
//	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
//	d3dPipelineStateDesc.BlendState = CreateBlendState();
//	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
//	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
//	d3dPipelineStateDesc.SampleMask = UINT_MAX;
//	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
//	d3dPipelineStateDesc.NumRenderTargets = 1;
//	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	d3dPipelineStateDesc.SampleDesc.Count = 1;
//	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
//
//	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);
//
//	d3dPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
//	hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);
//
//	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
//	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
//	if (pd3dHullShaderBlob) pd3dHullShaderBlob->Release();
//	if (pd3dDomainShaderBlob) pd3dDomainShaderBlob->Release();
//
//	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
//}
//
//void CTerrainTessellationShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
//{
//	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
//	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState((::gbTerrainTessellationWireframe) ? m_ppd3dPipelineStates[1] : m_ppd3dPipelineStates[0]);
//	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
//
//	UpdateShaderVariables(pd3dCommandList);
//}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
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

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSSkyBox", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "PSSkyBox", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardShader::CStandardShader()
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
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
	//return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	//return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
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
	//m_nObjects = (xObjects * yObjects * zObjects);  //97
	// m_nObjects += 1 + 2 * BULLETCNT + 1 + 4;
	//m_nObjects += 1 + 2 * BULLETCNT + 1 + 4 + MAX_NPC + MAX_USER;
	m_nObjects = 50;

#define TEXTURES 11
	CTexture* pTexture[TEXTURES];
	pTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/hp.dds", RESOURCE_TEXTURE2D, 0);   //여기 

	pTexture[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tree02.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mp.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[3] = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/snow.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/guard.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[5] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/water.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/metal.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[7] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[7]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/wind.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[8] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[8]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/fire.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[9] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[9]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tree.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[10] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[10]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/earth.dds", RESOURCE_TEXTURE2D, 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	//CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 11);//8
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	//for (int i = 0; i < TEXTURES; i++) CreateShaderResourceViews(pd3dDevice, pTexture[i], 0, 3);
	//for (int i = 0; i < TEXTURES; i++) CreateShaderResourceViews(pd3dDevice, pTexture[i], 0, 3);

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


	CTexturedRectMesh* part = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 10, 10, 0.0f);  //파티클 

	bullet = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 3.0f, 3.0f, 3.0f);

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

	// 50개
	m_ppObjects = new CGameObject * [m_nObjects];
	ZeroMemory(m_ppObjects, 50 * sizeof(CGameObject*));

//	CBillboardObject* pBillboardObject = NULL;
//
//	CGameObject* phouseObject = new CGameObject(1);
//
//	CBulletObject* bulletmesh;
//	for (int i = 1; i < 1 + BULLETCNT; ++i) {
//		bulletmesh = new CBulletObject(1);
//		bulletmesh->SetMesh(0, bullet);
//		bulletmesh->SetMaterial(pMaterials[7]);
//
//		bulletmesh->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
//		m_ppObjects[i] = bulletmesh;
//	}
//
//	for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i) {
//		pBillboardObject = new CBillboardObject(1);
//		pBillboardObject->SetMesh(0, part);
//#ifndef _WITH_BATCH_MATERIAL
//		pBillboardObject->SetMaterial(pMaterials[8]);
//#endif
//		pBillboardObject->SetPosition(0, -100, 0);
//		pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
//		m_ppObjects[i] = pBillboardObject;
//	}
//
//	std::default_random_engine dre;
//	std::uniform_int_distribution<> uid{ 0,2 };
//
//	int tmp;
//	int num = 0;
//	for (int i = 1 + 2 * BULLETCNT, x = 0; x < xObjects; x++)
//	{
//		for (int z = 0; z < zObjects; z++)
//		{
//			for (int y = 0; y < yObjects; y++)
//			{
//				if (i == 201) {
//					pBillboardObject = new CBillboardObject(1);
//					pBillboardObject->SetMesh(0, pRectMesh);
//				}
//				else if (i == 202) {
//					pBillboardObject = new CBillboardObject(1);
//					pBillboardObject->SetMesh(0, pRectMesh);
//				}
//				else if ((811 < i) && (i < 992)) { // npc hp
//					pBillboardObject = new CBillboardObject(1);
//					pBillboardObject->SetMesh(0, pRectMesh);
//				}
//				else {
//					pBillboardObject = new CBillboardObject(1);
//					pBillboardObject->SetMesh(0, pRectMesh);
//				}
//#ifndef _WITH_BATCH_MATERIAL    
//
//				if (i == 201) //HP
//					pBillboardObject->SetMaterial(pMaterials[1]);  //여기
//				else if (i == 202) { // mp
//
//					pBillboardObject->SetMaterial(pMaterials[1]);
//
//				}
//				else if ((811 < i) && (i < 992)) { // npc hp
//					pBillboardObject->SetMaterial(pMaterials[0]);
//				}
//				else {
//					pBillboardObject->SetMaterial(pMaterials[1]);
//				}
//				//	pBillboardObject->SetMaterial(pMaterials[uid(dre)]);
//
//#endif
//				// 장애물 인덱스 생각(기윤)
//				float xPosition;
//				float zPosition;
//				if (x * 97 + z >= 609) {
//					xPosition = 0.0f;
//					zPosition = 0.0f;
//				}
//				else {
//					xPosition = obstacles[x * 97 + z]._x + 100;
//					zPosition = obstacles[x * 97 + z]._z + 300;
//				}
//
//				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
//				//cout << xPosition << " " << fHeight << " " << zPosition << endl;
//			//	if (xPosition <= fTerrainWidth / 2 - 200 || xPosition >= fTerrainWidth / 2 + 200 ||   //나무 위치     
//				//	zPosition <= fTerrainLength / 2 - 200 || zPosition >= fTerrainLength / 2 + 200) {
//				pBillboardObject->SetPosition(xPosition, fHeight + 23, zPosition);         //1028 168 1028
//				if (pBillboardObject->GetPosition().y >= 50) {
//					pBillboardObject->SetPosition(xPosition, 23, zPosition);
//				}
//
//				pBillboardObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
//				m_ppObjects[i++] = pBillboardObject;
//
//				tmp = i;
//			}
//			num += 1;
//			if (num > 9)
//				num = 9;
//		}
//
//	}
//
	//// 플레이어
	//for (int i = m_nObjects - MAX_NPC - MAX_USER; i < m_nObjects - MAX_NPC; i++) {
	//	CTerrainPlayer* pOtherPlayer = new CTerrainPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
	//	pOtherPlayer->SetMesh(0, pOtherPlayerMesh[2]);
	//	pOtherPlayer->SetPosition(XMFLOAT3(0, -100, 0));
	//	pOtherPlayer->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	//	m_ppObjects[i] = pOtherPlayer;
	//}

	for (int i = 0; i < m_nObjects; i++) {
		CTerrainPlayer* pOtherPlayer = new CTerrainPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
		pOtherPlayer->SetMesh(0, pOtherPlayerMesh[2]);
		pOtherPlayer->SetPosition(XMFLOAT3(0, -100, 0));
		//pOtherPlayer->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
		m_ppObjects[i] = pOtherPlayer;
		m_ppObjects[i]->OnPrepareAnimate();
	}
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//// NPC
	//for (int i = m_nObjects - MAX_NPC; i < m_nObjects; i++) {
	//	CTerrainPlayer* pNpc = new CTerrainPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
	//	pNpc->SetMesh(0, pOtherPlayerMesh[1]);
	//	pNpc->SetPosition(XMFLOAT3(0, -100, 0));
	//	pNpc->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	//	m_ppObjects[i] = pNpc;
	//}

}

void CObjectsShader::BuildObjects_Raid(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
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
	m_nObjects += 1 + 2 * BULLETCNT + 1 + 4 + MAX_NPC + MAX_USER;
	/*-------------------------------------------------------------
	//m_nObjects = xObjects * yObjects * zObjects = 9400
	m_nObjects += 1
	m_nObjects += 2*BULLETCNT	// 투사체 처리
	m_nObjects += 1
	m_nObjects += 1
	m_nObjects += GAIA_ROOM		// Party Player
	m_nObjects += 1				// GAIA(NPC)

	m_nObjects = 1+200+1+1+4+1 = 208
	-------------------------------------------------------------*/


#define TEXTURES 14
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
	pTexture[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/snow.dds", RESOURCE_TEXTURE2D, 0);

	pTexture[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/mirror.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[7] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[7]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/guard.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[8] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[8]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/water.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[9] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[9]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/metal.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[10] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[10]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/wind.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[11] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[11]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/fire.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[12] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[12]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tree.dds", RESOURCE_TEXTURE2D, 0);


	pTexture[13] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture[13]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/earth.dds", RESOURCE_TEXTURE2D, 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 14);//8
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	for (int i = 0; i < TEXTURES; i++) CreateShaderResourceViews(pd3dDevice, pTexture[i], 0, 1);

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


	CTexturedRectMesh* part = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 10, 10, 0.0f);  //파티클 

	bullet = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 3.0f, 3.0f, 3.0f);

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

	CBulletObject* bulletmesh;
	for (int i = 1; i < 1 + BULLETCNT; ++i) {
		bulletmesh = new CBulletObject(1);
		bulletmesh->SetMesh(0, bullet);
		//	bulletmesh->SetMaterial(pMaterials[7]);

		bulletmesh->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = bulletmesh;
	}

	for (int i = 1 + BULLETCNT; i < 1 + 2 * BULLETCNT; ++i) {
		pBillboardObject = new CBillboardObject(1);
		pBillboardObject->SetMesh(0, part);
#ifndef _WITH_BATCH_MATERIAL
		pBillboardObject->SetMaterial(pMaterials[8]);
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
					pBillboardObject->SetMesh(0, pRectMesh);
				}
				else if (i == 202) {
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, pRectMesh);
				}
				else if ((811 < i) && (i < 992)) { // npc hp
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, pRectMesh);
				}
				else {
					pBillboardObject = new CBillboardObject(1);
					pBillboardObject->SetMesh(0, pRectMesh);
				}
#ifndef _WITH_BATCH_MATERIAL    

				if (i == 201) //HP
					pBillboardObject->SetMaterial(pMaterials[1]);  //여기
				else if (i == 202) { // mp

					pBillboardObject->SetMaterial(pMaterials[1]);

				}
				else if ((811 < i) && (i < 992)) { // npc hp
					pBillboardObject->SetMaterial(pMaterials[0]);
				}
				else {
					pBillboardObject->SetMaterial(pMaterials[1]);
				}
				//	pBillboardObject->SetMaterial(pMaterials[uid(dre)]);

#endif
				// 장애물 인덱스 생각(기윤)
				float xPosition = 0;
				float zPosition = 0;
				if (x * 97 + z >= 609) {
					xPosition = 0.0f;
					zPosition = 0.0f;
				}
				else {
					xPosition = 2024;
					zPosition = 2024;
				}

				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				//cout << xPosition << " " << fHeight << " " << zPosition << endl;
			//	if (xPosition <= fTerrainWidth / 2 - 200 || xPosition >= fTerrainWidth / 2 + 200 ||   //나무 위치     
				//	zPosition <= fTerrainLength / 2 - 200 || zPosition >= fTerrainLength / 2 + 200) {
				pBillboardObject->SetPosition(xPosition, fHeight + 23, zPosition);         //1028 168 1028
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

	//// 플레이어
	//for (int i = m_nObjects - MAX_NPC - MAX_USER; i < m_nObjects - MAX_NPC; i++) {
	//	CTerrainPlayer* pOtherPlayer = new CTerrainPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
	//	pOtherPlayer->SetMesh(0, pOtherPlayerMesh[2]);
	//	pOtherPlayer->SetPosition(XMFLOAT3(0, -100, 0));
	//	pOtherPlayer->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	//	m_ppObjects[i] = pOtherPlayer;
	//}

	//// NPC
	//for (int i = m_nObjects - MAX_NPC; i < m_nObjects; i++) {
	//	CTerrainPlayer* pNpc = new CTerrainPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain);
	//	pNpc->SetMesh(0, pOtherPlayerMesh[1]);
	//	pNpc->SetPosition(XMFLOAT3(0, -100, 0));
	//	pNpc->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	//	m_ppObjects[i] = pNpc;
	//}
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
	CTerrainPlayer* pPlayer = NULL;
	int server_id = MAX_USER + MAX_NPC;
	int MAX_WORLD_SHADER = m_nObjects - server_id;

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] == nullptr) continue;
		if (j >= m_nObjects - server_id) {
			pPlayer = reinterpret_cast<CTerrainPlayer*>(m_ppObjects[j]);
			bool tp = pPlayer->GetUse();
			pPlayer->SetUse(get_use_to_server(j - MAX_WORLD_SHADER));
			if (pPlayer->GetUse()) {

				if (tp != pPlayer->GetUse()) {
					// 최초 정보 불러오기 및 종족에 맞게 변환
					get_basic_information(pPlayer, j - MAX_WORLD_SHADER);
					if (pPlayer->m_tribe == HUMAN) {
						pPlayer->SetMesh(0, pOtherPlayerMesh[2]);
						get_player_information(pPlayer, j - MAX_WORLD_SHADER);
					}
					else if (pPlayer->m_tribe == BOSS) {
						pPlayer->SetMesh(0, pOtherPlayerMesh[0]);
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

				reinterpret_cast<CTerrainPlayer*>(m_ppObjects[j])->m_hp = get_hp_to_server(j - MAX_WORLD_SHADER);
				m_ppObjects[j]->SetPosition(get_position_to_server(j - MAX_WORLD_SHADER));
				if (j >= 10615 && j < 10795) {
					//m_ppObjects[j]->SetPosition(get_position_to_server(j - 9614));
					m_ppObjects[j]->vCenter = { m_ppObjects[j]->GetPosition().x, m_ppObjects[j]->GetPosition().y + 10, m_ppObjects[j]->GetPosition().z };
				}

				//cout << j << endl;
				//cout << m_ppObjects[j]->vCenter.x << m_ppObjects[j]->vCenter.y << m_ppObjects[j]->vCenter.z << endl;

				pPlayer->SetLook(get_look_to_server(j - MAX_WORLD_SHADER));
				pPlayer->Animate(pTimer, pCamera, m_ppObjects[j]);

				// hp bar 렌더링
				//int hp_j = j - MAX_WORLD_SHADER - 1000 + 812;
				//m_ppObjects[hp_j]->Animate2(hp_j,pTimer, pCamera, pPlayer);
			}
			else {
				m_ppObjects[j]->SetPosition(XMFLOAT3(0, -100, 0));
				pPlayer->Animate(pTimer, pCamera, m_ppObjects[j]);
			}
		}
		else if (j <= bulletidx) { //총알 원위치 
			if (m_ppObjects[j]->GetPosition().x < 0 || m_ppObjects[j]->GetPosition().x>5000 ||
				m_ppObjects[j]->GetPosition().z < 0 || m_ppObjects[j]->GetPosition().z>5000) {
				m_ppObjects[j]->SetPosition(0, 100, 0);

			}

			else if (reinterpret_cast<CBulletObject*>(m_ppObjects[j])->Animate(pTimer, pCamera, player, map)) { // 구체 이펙트 


				if (start[j] == 0)
					start[j] = pTimer.GetTotalTime();


				//m_ppObjects[j + BULLETCNT]->SetPosition(effect_x, effect_y + 10, effect_z);

				m_ppObjects[j + BULLETCNT]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 0);


				if (j < 70) {

					//m_ppObjects[j + BULLETCNT + 30]->SetPosition(effect_x - 10, effect_y, effect_z);
					//m_ppObjects[j + BULLETCNT + 31]->SetPosition(effect_x, effect_y , effect_z);
					//m_ppObjects[j + BULLETCNT + 32]->SetPosition(effect_x - 10, effect_y , effect_z);

					m_ppObjects[j + BULLETCNT + 32]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 1);
					m_ppObjects[j + BULLETCNT + 31]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 2);
					m_ppObjects[j + BULLETCNT + 30]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 3);



				}
				else {

					//m_ppObjects[j + BULLETCNT - 30]->SetPosition(effect_x - 10, effect_y + 10, effect_z);
					//m_ppObjects[j + BULLETCNT - 31]->SetPosition(effect_x, effect_y , effect_z);
					//m_ppObjects[j + BULLETCNT - 32]->SetPosition(effect_x - 10, effect_y + 10, effect_z);

					//m_ppObjects[j + BULLETCNT - 32]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 1);
					//m_ppObjects[j + BULLETCNT - 31]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 2);
					//m_ppObjects[j + BULLETCNT - 30]->AnimatePart(pTimer, start[j], XMFLOAT3(effect_x, effect_y + 10, effect_z), 3);
				}


				if (pTimer.GetTotalTime() - start[j] >= 2) {
					hit_check = false;
					start[j] = 0;
				}
			}
		}
		else if (j == m_nObjects - 2) {
			m_ppObjects[j]->SyncPlayer(pTimer, pCamera, player);
		}
		else if (j >= 812 && j < 992) {
			pPlayer = reinterpret_cast<CTerrainPlayer*>(m_ppObjects[MAX_WORLD_SHADER + MAX_USER + j - 812]);
			if (pPlayer->GetUse()) {
				int width = ((float)pPlayer->m_hp / pPlayer->m_max_hp) * 50.0f;
				m_ppObjects[j]->SetMesh(0, newhp[width]);
			}
			m_ppObjects[j]->Animate2(j, pTimer, pCamera, pPlayer);
		}
		else {
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
		for (int j = 0; j < m_nObjects; j++) { 
			if (m_ppObjects[j]) { 
				m_ppObjects[j]->ReleaseUploadBuffers();
			}
		}
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

	for (int j = 0; j < m_nObjects/* - 4*/; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
	pd3dCommandList->OMSetStencilRef(1);
	if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[1]);

	//if (m_ppObjects[m_nObjects - 1]) m_ppObjects[m_nObjects - 1]->Render(pd3dCommandList, pCamera);  //거울 스텐실에

	//if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[2]);

	//for (int j = m_nObjects - 4; j < m_nObjects - 1; j++)
	//{
	//	if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);   //반사 객체
	//}

	//if (m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[3]);

	//if (m_ppObjects[m_nObjects - 1]) m_ppObjects[m_nObjects - 1]->Render(pd3dCommandList, pCamera);  //거울
}

void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature) {
	if (pd3dGraphicsRootSignature)
	{
		m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
		pd3dGraphicsRootSignature->AddRef();
	}
	m_nPipelineStates = 3;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//거울
	//d3dPipelineStateDesc.RasterizerState.FrontCounterClockwise = true;

	//HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);

	//HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);


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

	//hResult = pd3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[2]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader()
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardObjectsShader::CStandardObjectsShader()
{
}

CStandardObjectsShader::~CStandardObjectsShader()
{
}

void CStandardObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CStandardObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CStandardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CStandardObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CStandardObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CStandardShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHellicopterObjectsShader::CHellicopterObjectsShader()
{
}

CHellicopterObjectsShader::~CHellicopterObjectsShader()
{
}

float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
    float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
    xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
    xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
    xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}

void CHellicopterObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	m_nObjects = 40;
	m_ppObjects = new CGameObject*[m_nObjects];

	CLoadedModelInfo *pSuperCobraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SuperCobra.bin", this);
	CLoadedModelInfo *pGunshipModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gunship.bin", this);

	int nColumnSpace = 5, nColumnSize = 30;           
    int nFirstPassColumnSize = (m_nObjects % nColumnSize) > 0 ? (nColumnSize - 1) : nColumnSize;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	int nObjects = 0;
    for (int h = 0; h < nFirstPassColumnSize; h++)
    {
        for (int i = 0; i < floor(float(m_nObjects) / float(nColumnSize)); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			float fHeight = pTerrain->GetHeight(390.0f, 670.0f);
			XMFLOAT3 xmf3Position = RandomPositionInSphere(XMFLOAT3(390.0f, fHeight + 35.0f, 670.0f), Random(20.0f, 100.0f), h - int(floor(nColumnSize / 2.0f)), nColumnSpace);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z) + Random(0.0f, 25.0f);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
		}
    }

    if (nFirstPassColumnSize != nColumnSize)
    {
        for (int i = 0; i < m_nObjects - int(floor(float(m_nObjects) / float(nColumnSize)) * nFirstPassColumnSize); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			m_ppObjects[nObjects]->SetPosition(RandomPositionInSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), Random(20.0f, 100.0f), nColumnSize - int(floor(nColumnSize / 2.0f)), nColumnSpace));
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
        }
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pSuperCobraModel) delete pSuperCobraModel;
	if (pGunshipModel) delete pGunshipModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAngrybotObjectsShader::CAngrybotObjectsShader()
{
}

CAngrybotObjectsShader::~CAngrybotObjectsShader()
{
}

void CAngrybotObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	CLoadedModelInfo *pAngrybotModel = pModel;
	if (!pAngrybotModel) pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CAngrybotObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pAngrybotModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 2));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, (nObjects % 2) ? 0.25f : 1.0f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 3) ? 0.85f : 0.0f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch*x + 390.0f, 0.0f, 730.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects++]->SetScale(2.0f, 2.0f, 2.0f);
		}
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pAngrybotModel) delete pAngrybotModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CEthanObjectsShader::CEthanObjectsShader()
{
}

CEthanObjectsShader::~CEthanObjectsShader()
{
}

void CEthanObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	CLoadedModelInfo *pEthanModel = pModel;
	if (!pEthanModel) pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pEthanModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 4));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.025f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 10) * 0.35f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch*x + 290.0f, 0.0f, 750.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects++]->SetPosition(xmf3Position);
		}
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pEthanModel) delete pEthanModel;
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
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "VSPlayer", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CPlayerShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"b_Shaders.hlsl", "PSPlayer", "ps_5_1", ppd3dShaderBlob));
}

void CPlayerShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}