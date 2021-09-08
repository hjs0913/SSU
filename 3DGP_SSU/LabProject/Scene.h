#pragma once
#include "Timer.h"
#include "Shader.h"
#include "Camera.h"
#include "GameObject.h"

class CGameObject;
class CObjectsShader;
class CMaterial;

struct LIGHT
{
	XMFLOAT4 m_xmf4Ambient; XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular;
	XMFLOAT3 m_xmf3Position;
	float m_fFalloff;
	XMFLOAT3 m_xmf3Direction;
	float m_fTheta; //cos(m_fTheta)
	XMFLOAT3 m_xmf3Attenuation;
	float m_fPhi; //cos(m_fPhi)
	bool m_bEnable;
	int m_nType;
	float m_fRange;
	float padding;
};

struct LIGHTS
{
	LIGHT m_pLights[MAX_LIGHTS];
	XMFLOAT4 m_xmf4GlobalAmbient;
};

struct MATERIALS
{
	MATERIAL m_pReflections[MAX_MATERIALS];
};

class CScene
{
public:
	CScene();
	~CScene();

	//씬에서 마우스와 키보드 메시지를 처리한다.
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();
	//void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);

	void Render(ID3D12GraphicsCommandList* pd3dCommandListt, CCamera* pCamera);

	void ReleaseUploadBuffers();

protected:
	//씬은 셰이더들의 집합이다. 셰이더들은 게임 객체들의 집합이다. 
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	//배치(Batch) 처리를 하기 위하여 씬을 셰이더들의 리스트로 표현한다. 
	CObjectsShader *m_pShaders = NULL;
	int m_nShaders = 0;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	//루트 시그너쳐를 나타내는 인터페이스 포인터이다.


public:
	//씬의 모든 조명과 재질을 생성
	void BuildLightsAndMaterials();
	//씬의 모든 조명과 재질을 위한 리소스를 생성하고 갱신
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

public:
	CPlayer* m_pPlayer = NULL;

	//씬의 조명
	LIGHTS* m_pLights = NULL;
	//조명을 나타내는 리소스와 리소스에 대한 포인터이다. 
	ID3D12Resource *m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	//씬의 객체들에 적용되는 재질
	MATERIALS* m_pMaterials = NULL;
	//재질을 나타내는 리소스와 리소스에 대한 포인터이다. 
	ID3D12Resource *m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;
};


