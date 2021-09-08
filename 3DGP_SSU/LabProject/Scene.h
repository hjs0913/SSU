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

	//������ ���콺�� Ű���� �޽����� ó���Ѵ�.
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);

	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�. 
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
	//���� ���̴����� �����̴�. ���̴����� ���� ��ü���� �����̴�. 
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	//��ġ(Batch) ó���� �ϱ� ���Ͽ� ���� ���̴����� ����Ʈ�� ǥ���Ѵ�. 
	CObjectsShader *m_pShaders = NULL;
	int m_nShaders = 0;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	//��Ʈ �ñ׳��ĸ� ��Ÿ���� �������̽� �������̴�.


public:
	//���� ��� ����� ������ ����
	void BuildLightsAndMaterials();
	//���� ��� ����� ������ ���� ���ҽ��� �����ϰ� ����
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

public:
	CPlayer* m_pPlayer = NULL;

	//���� ����
	LIGHTS* m_pLights = NULL;
	//������ ��Ÿ���� ���ҽ��� ���ҽ��� ���� �������̴�. 
	ID3D12Resource *m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	//���� ��ü�鿡 ����Ǵ� ����
	MATERIALS* m_pMaterials = NULL;
	//������ ��Ÿ���� ���ҽ��� ���ҽ��� ���� �������̴�. 
	ID3D12Resource *m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;
};


