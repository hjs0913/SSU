//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Timer.h"

//#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") //콘솔 띄우자 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_net_skill_animation[3] = { false };
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		
		Move(xmf3Shift, dwDirection, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, int dir, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		switch (dir) {
		case DIR_FORWARD:
			SetMaxVelocityXZ(45.0f);
			break;
		case DIR_BACKWARD:
		case DIR_RIGHT:
		case DIR_LEFT:
			SetMaxVelocityXZ(30.0f);
			break;
		default:
			break;
		}

		SetMaxVelocityXZ(150.0f);
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();
	
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::Attack(bool isAttack)
{

}

void CPlayer::Skill(int n)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo *pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", NULL);
	SetChild(pModel->m_pModelRootObject, true);

	OnPrepareAnimate();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pModel) delete pModel;
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CPlayer::Animate(fTimeElapsed);
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

CCamera *CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(2.5f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(100.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(40.0f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(20.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(25.5f);
			SetMaxVelocityY(20.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -30.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 2000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void *pCallbackData, float fTrackPosition)
{
   _TCHAR *pWavName = (_TCHAR *)pCallbackData; 
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
   PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
   PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, JOB job)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	CLoadedModelInfo* pAngrybotModel = NULL;
	switch (job) {
	case J_DILLER: {
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bastard_Warrior.bin", NULL);
		SetChild(pAngrybotModel->m_pModelRootObject, true);
		anim_cnt = pAngrybotModel->m_pAnimationSets->m_nAnimationSets;
		SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

		break;
	}
	case J_TANKER: {
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mighty_Warrior.bin", NULL);
		SetChild(pAngrybotModel->m_pModelRootObject, true);
		anim_cnt = pAngrybotModel->m_pAnimationSets->m_nAnimationSets;
		SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

		break;
	}
	case J_SUPPORTER: {
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Priestess.bin", NULL);
		SetChild(pAngrybotModel->m_pModelRootObject, true);
		anim_cnt = pAngrybotModel->m_pAnimationSets->m_nAnimationSets;
		SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

		break;
	}
	case J_MAGICIAN: {
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Wizard_Girl.bin", NULL);
		SetChild(pAngrybotModel->m_pModelRootObject, true);
		anim_cnt = pAngrybotModel->m_pAnimationSets->m_nAnimationSets;
		SetScale(XMFLOAT3(12.0f, 12.0f, 12.0f));

		break;
	}
	default: {
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bastard_Warrior.bin", NULL);
		SetChild(pAngrybotModel->m_pModelRootObject, true);
		anim_cnt = pAngrybotModel->m_pAnimationSets->m_nAnimationSets;
		break;
	}
	}

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, anim_cnt, pAngrybotModel, true);
	m_pSkinnedAnimationController->m_pAnimationSets = pAngrybotModel->m_pAnimationSets;

	for (int i = 0; i < anim_cnt; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	}
	m_pSkinnedAnimationController->SetTrackEnable(0, true);

	// 2번 부터 ANIMATION_TYPE_ONCE
	for (int i = 2; i < anim_cnt; ++i) {
		m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i]->m_nType = ANIMATION_TYPE_ONCE;
	}

	/*if (job == J_TANKER) {
		m_pSkinnedAnimationController->m_pAnimationTracks[5].m_fSpeed = 0.3f;
	}*/


	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0.1f, _T("NormalAttack"));
	//m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.000f, _T("Sound/발자국2.wav"));
	//m_pSkinnedAnimationController->SetCallbackKey(2, 1, 0.125f, _T("Sound/일반공격.wav"));
	//m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	
	SetPosition(XMFLOAT3(3500.0f, pTerrain->GetHeight(3500.0f, 590.0f), 590.0f));

	//SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));
	if (pAngrybotModel) delete pAngrybotModel;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(50.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 30.0f, -70.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0,0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable || 
		m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable ||
		m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable ||
		m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable ||
		m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable
		) return;
	m_pSkinnedAnimationController->SetTrackAllDisable();
	m_pSkinnedAnimationController->SetTrackEnable(1, true);
	/*m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fPosition = 0.0f;
	m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3]->m_fPosition = 0.0f;*/

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

	if (m_pSkinnedAnimationController)
	{
		if (m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
			return;
		}

		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (/*::IsZero(fLength)*/!dwDir)	// 가만히 서있을 때
		{
			m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_pSkinnedAnimationController->SetTrackEnable(1, false);

			if (m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {	// 일반 공격 시
				m_pSkinnedAnimationController->SetTrackEnable(0, false);
				m_pSkinnedAnimationController->SetTrackEnable(3, false);
				
			}

			if (m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable
				|| m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable
				|| m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable) {	// 주변 공격 시
				m_pSkinnedAnimationController->SetTrackEnable(0, false);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
			}

			if (m_pSkinnedAnimationController->m_pAnimationTracks[0].m_bEnable) {
				for (int i = 2; i < anim_cnt; ++i) {
					m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[i]->m_fPosition = 0.0f;
				}
			}
		}

		if (m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable) {
			float playTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fLength - m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fPosition;

			if (playTime <= 0.049)
				Damage_On = false;
			/*if (playTime <= 0.049) {
				m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[2]->m_fPosition = 0.0f;
				m_pSkinnedAnimationController->SetTrackEnable(0, true);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
			}*/
		}

		if (m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable) {
			float playTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3]->m_fLength - m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3]->m_fPosition;
			if (playTime <= 0.049) {
				m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[3]->m_fPosition = 0.0f;
				m_pSkinnedAnimationController->SetTrackEnable(0, true);
				m_pSkinnedAnimationController->SetTrackEnable(3, false);
			}
		}

		if (m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable) {
			float playTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fLength - m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition;
			if (playTime <= 0.049) {
				/*m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition = 0.0f;
				m_pSkinnedAnimationController->SetTrackEnable(0, true);
				m_pSkinnedAnimationController->SetTrackEnable(4, false);*/
			}
		}

		if (m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable) {
			float playTime = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[5]->m_fLength - m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[5]->m_fPosition;
			if (playTime <= 0.049) {
				m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[5]->m_fPosition = 0.0f;
				m_pSkinnedAnimationController->SetTrackEnable(0, true);
				m_pSkinnedAnimationController->SetTrackEnable(5, false);
			}
		}
	}



}

void CTerrainPlayer::Attack(bool isAttack)
{
	m_isAttack = isAttack;
	CPlayer::Attack(isAttack);
	if (isAttack) {
		m_pSkinnedAnimationController->SetTrackAllDisable();
		m_pSkinnedAnimationController->SetTrackEnable(2, true);
		m_pSkinnedAnimationController->SetCallbackKeys(2, 3);
#ifdef _WITH_SOUND_RESOURCE
		m_pSkinnedAnimationController->SetCallbackKey(1, 0.1f, _T("NormalAttack"));
#else
		m_pSkinnedAnimationController->SetCallbackKey(2, 1, 0.125f, _T("Sound/일반공격.wav"));
#endif
		CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	}
}

void CTerrainPlayer::Skill(int n)
{
	if (m_pSkinnedAnimationController->m_pAnimationTracks[6].m_bEnable) {
		return;
	}

	CPlayer::Skill(n);
	m_pSkinnedAnimationController->SetTrackAllDisable();
	if (n == 0) {
		m_pSkinnedAnimationController->SetTrackEnable(3, true);
		m_pSkinnedAnimationController->SetCallbackKeys(3, 4);
#ifdef _WITH_SOUND_RESOURCE
		m_pSkinnedAnimationController->SetCallbackKey(2, 0.1f, _T("SKILL1"));
#else
		switch (my_job)
		{
		case J_DILLER:
			m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/전사 1번 스킬.wav"));
			break;
		case J_TANKER:
			m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/탱커 1번 스킬.wav"));
			break;
		case J_MAGICIAN:
			m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/마법사 1번 스킬.wav"));
			break;
		case J_SUPPORTER:
			m_pSkinnedAnimationController->SetCallbackKey(3, 2, 0.125f, _T("Sound/서포터 1번 스킬.wav"));
			break;
		}
#endif
		CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	}
	else if (n == 1) {
		//m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[4]->m_fPosition = 0.0f;
		m_pSkinnedAnimationController->SetTrackEnable(4, true);
		m_pSkinnedAnimationController->SetCallbackKeys(4, 5);
#ifdef _WITH_SOUND_RESOURCE
		m_pSkinnedAnimationController->SetCallbackKey(3, 0.1f, _T("SKILL2"));
#else
		switch (my_job)
		{
		case J_DILLER:
			m_pSkinnedAnimationController->SetCallbackKey(4, 3, 0.125f, _T("Sound/전사 2번 스킬.wav"));
			break;
		case J_TANKER:
			m_pSkinnedAnimationController->SetCallbackKey(4, 3, 0.125f, _T("Sound/탱커 2번 스킬.wav"));
			break;
		case J_MAGICIAN:
			m_pSkinnedAnimationController->SetCallbackKey(4, 3, 0.125f, _T("Sound/마법사 2번 스킬.wav"));
			break;
		case J_SUPPORTER:
			m_pSkinnedAnimationController->SetCallbackKey(4, 3, 0.125f, _T("Sound/서포터 2번 스킬.wav"));
			break;
		}
#endif
		CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	}
	else if (n == 2) {
		m_pSkinnedAnimationController->SetTrackEnable(5, true);
		m_pSkinnedAnimationController->SetCallbackKeys(5, 6);
#ifdef _WITH_SOUND_RESOURCE
		m_pSkinnedAnimationController->SetCallbackKey(4, 0.1f, _T("SKILL3"));
#else
		switch (my_job)
		{

		case J_DILLER:
			m_pSkinnedAnimationController->SetCallbackKey(5, 4, 0.125f, _T("Sound/전사 3번 스킬.wav"));
			break;
		case J_TANKER:
			m_pSkinnedAnimationController->SetCallbackKey(5, 4, 0.125f, _T("Sound/탱커 3번 스킬.wav"));
			break;
		case J_MAGICIAN:
			m_pSkinnedAnimationController->SetCallbackKey(5, 4, 0.125f, _T("Sound/서포터 3번 스킬.wav"));
			break;
		case J_SUPPORTER:
			m_pSkinnedAnimationController->SetCallbackKey(5, 4, 0.125f, _T("Sound/서포터 3번 스킬.wav"));
			break;
		}
#endif
		CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);
	}
}

void CTerrainPlayer::ChangeAnimationState(Player_Animation animState)
{
	m_animState = animState;
	for (int i = 0; i < anim_cnt; ++i) {
		if (i == m_animState) {
			m_pSkinnedAnimationController->SetTrackEnable(i, true);
			continue;
		}
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
}
