#include "SceneContext.h"
#include "SetCamera.h"
#include "Common/Common.h"

extern void AnimateFbxNodeHierarchy(FbxNode *pfbxNode, FbxTime& rfbxTime);
//extern void RenderFbxNodeHierarchy(FbxNode *pfbxNode, FbxAMatrix& fbxmtxWorld);
extern void RenderFbxNodeHierarchy(FbxNode *pfbxNode, FbxTime& rfbxTime, FbxAMatrix& fbxmtxWorld);

//#define SAMPLE_FBX_FILENAME	"Angrybot.fbx"
//#define SAMPLE_FBX_FILENAME	"Monster.fbx"
//#define SAMPLE_FBX_FILENAME	"humanoid.fbx"
//#define SAMPLE_FBX_FILENAME	"Dude.fbx"
//#define SAMPLE_FBX_FILENAME	"DandyTellMe.fbx"
#define SAMPLE_FBX_FILENAME	"Bastard_Warrior_Idle.fbx"

CSceneContext::CSceneContext()
{
   InitializeSdkObjects(m_pfbxSdkManager, m_pfbxScene);

   if (LoadScene(m_pfbxSdkManager, m_pfbxScene, SAMPLE_FBX_FILENAME))
   {
	   FbxGeometryConverter fbxGeomConverter(m_pfbxSdkManager);
	   fbxGeomConverter.Triangulate(m_pfbxScene, true);

	   FbxAxisSystem fbxSceneAxisSystem = m_pfbxScene->GetGlobalSettings().GetAxisSystem();
	   FbxAxisSystem fbxOurAxisSystem(FbxAxisSystem::eOpenGL);
	   if (fbxSceneAxisSystem != fbxOurAxisSystem) fbxOurAxisSystem.ConvertScene(m_pfbxScene);

	   m_pfbxScene->FillAnimStackNameArray(m_fbxAnimationStackNameArray);
	   SetCurrentAnimStack(0);

	   //m_fbxFrameTime.SetTime(0, 0, 0, 1, 0, FbxTime::eDefaultMode/*m_pfbxScene->GetGlobalSettings().GetTimeMode()*/);
	   //m_fbxFrameTime.SetTime(0, 0, 0, 1, 0, 0, FbxTime::eDefaultMode);
	   //m_fbxFrameTime.SetMilliSeconds(16);
	   	float fMilliSecs = 0.01666f * 1000.0f;
		int nFrames = int(fMilliSecs);
		float fResidual = (fMilliSecs - nFrames) * 10.0f;
		int nFields = int(fResidual);
		int nResidual = int((fResidual - nFields) * 10.0f);
	   //m_fbxFrameTime.SetTime(0, 0, 0, nFrames, nFields, nResidual, FbxTime::eFrames1000);

	   m_fbxFrameTime.SetSecondDouble(0.01666f);
   }
}

CSceneContext::~CSceneContext()
{
    FbxArrayDelete(m_fbxAnimationStackNameArray);
	DestroySdkObjects(m_pfbxSdkManager, true);
}

bool CSceneContext::SetCurrentAnimStack(int nAnimation)
{
	int nAnimationStacks = m_fbxAnimationStackNameArray.GetCount();
	if ((nAnimationStacks == 0) || (nAnimation >= nAnimationStacks)) return(false);

	FbxString *pfbxStackName = m_fbxAnimationStackNameArray[nAnimation];
	FbxAnimStack *pfbxAnimationStack = m_pfbxScene->FindMember<FbxAnimStack>(pfbxStackName->Buffer());
	if (!pfbxAnimationStack) return(false);

	m_pfbxCurrentAnimLayer = pfbxAnimationStack->GetMember<FbxAnimLayer>();
	m_pfbxScene->SetCurrentAnimationStack(pfbxAnimationStack);

	FbxTakeInfo *pfbxTakeInfo = m_pfbxScene->GetTakeInfo(*pfbxStackName);
	if (pfbxTakeInfo)
	{
		for (int i = 0; i < OBJECTS; i++)
		{
			m_pfbxStartTimes[i] = pfbxTakeInfo->mLocalTimeSpan.GetStart();
			m_pfbxStopTimes[i] = pfbxTakeInfo->mLocalTimeSpan.GetStop();
		}
	}
	else
	{
		FbxTimeSpan fbxTimeLineTimeSpan;
		m_pfbxScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(fbxTimeLineTimeSpan);
		for (int i = 0; i < OBJECTS; i++)
		{
			m_pfbxStartTimes[i] = fbxTimeLineTimeSpan.GetStart();
			m_pfbxStopTimes[i] = fbxTimeLineTimeSpan.GetStop();
		}
	}

	for (int i = 0; i < OBJECTS; i++)
	{
		FbxTime fbxTimeOffset;
		fbxTimeOffset.SetTime(0, 0, 0, i * 20, 0, m_pfbxScene->GetGlobalSettings().GetTimeMode());
		m_pfbxCurrentTimes[i] = m_pfbxStartTimes[i] + fbxTimeOffset;
	}

	return(true);
}

void CSceneContext::OnTimer() 
{
	for (int i = 0; i < OBJECTS; i++)
	{
		if (m_pfbxStopTimes[i] > m_pfbxStartTimes[i])
		{
			m_pfbxCurrentTimes[i] += m_fbxFrameTime;
			if (m_pfbxCurrentTimes[i] > m_pfbxStopTimes[i]) m_pfbxCurrentTimes[i] = m_pfbxStartTimes[i];
		}
	}
}

bool CSceneContext::OnDisplay()
{
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	::glPushAttrib(GL_ENABLE_BIT);
	::glPushAttrib(GL_LIGHTING_BIT);
	::glEnable(GL_DEPTH_TEST);
	::glEnable(GL_CULL_FACE);

	SetCamera(m_pfbxScene, m_pfbxCurrentTimes[0], m_pfbxCurrentAnimLayer, WINDOW_WIDTH, WINDOW_HEIGHT);

	FbxAMatrix fbxmtxWorld;
	if (OBJECTS >= 1)
	{
		fbxmtxWorld.SetIdentity();
		AnimateFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[0]);
		RenderFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[0], fbxmtxWorld);
	}
	if (OBJECTS >= 2)
	{
		fbxmtxWorld.SetIdentity();
		fbxmtxWorld.SetR(FbxVector4(0.0, -90.0, 0.0, 0.0));
		fbxmtxWorld.SetT(FbxVector4(280.0, 0.0, -50.0, 1.0));
		AnimateFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[1]);
		RenderFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[1], fbxmtxWorld);
	}
	if (OBJECTS >= 3)
	{
		fbxmtxWorld.SetIdentity();
		fbxmtxWorld.SetR(FbxVector4(0.0, -45.0, 0.0, 0.0));
		fbxmtxWorld.SetT(FbxVector4(180.0, 0.0, -150.0, 1.0));
		AnimateFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[2]);
		RenderFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[2], fbxmtxWorld);
	}
	if (OBJECTS >= 4)
	{
		fbxmtxWorld.SetIdentity();
		fbxmtxWorld.SetR(FbxVector4(0.0, 90.0, 0.0, 0.0));
		fbxmtxWorld.SetT(FbxVector4(-180.0, 0.0, -50.0, 1.0));
		AnimateFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[3]);
		RenderFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[3], fbxmtxWorld);
	}
	if (OBJECTS >= 5)
	{
		fbxmtxWorld.SetIdentity();
		fbxmtxWorld.SetR(FbxVector4(0.0, 45.0, 0.0, 0.0));
		fbxmtxWorld.SetT(FbxVector4(-280.0, 0.0, -100.0, 1.0));
		AnimateFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[4]);
		RenderFbxNodeHierarchy(m_pfbxScene->GetRootNode(), m_pfbxCurrentTimes[4], fbxmtxWorld);
	}

	fbxmtxWorld.SetIdentity();
	DisplayGrid(fbxmtxWorld);

	::glPopAttrib();
	::glPopAttrib();

	return(true);
}

void CSceneContext::OnMouseClick(int nButton, int nState, int xPos, int yPos)
{
	FbxCamera *pfbxCamera = GetCurrentCamera(m_pfbxScene);
	if (pfbxCamera)
	{
		m_fbxv4CameraPosition = pfbxCamera->Position.Get();
		m_fbxv4CameraCenter = pfbxCamera->InterestPosition.Get();
		m_fCameraRoll = pfbxCamera->Roll.Get();
	}
	m_xPreviousMousePos = xPos;
	m_yPreviousMousePos = yPos;

	switch (nButton)
	{
		case LEFT_BUTTON:
			switch (nState)
			{
				case BUTTON_DOWN:
					m_nCameraStatus = (m_nCameraStatus == CAMERA_ZOOM) ? CAMERA_PAN : CAMERA_ORBIT;
					break;
				default:
					m_nCameraStatus = (m_nCameraStatus == CAMERA_PAN) ? CAMERA_ZOOM : CAMERA_NOTHING;
					break;
			}
			break;
		case MIDDLE_BUTTON:
			switch (nState)
			{
				case BUTTON_DOWN:
					m_nCameraStatus = (m_nCameraStatus == CAMERA_ORBIT) ? CAMERA_PAN : CAMERA_ZOOM;
					break;
				default:
					m_nCameraStatus = (m_nCameraStatus == CAMERA_PAN) ? CAMERA_ORBIT : CAMERA_NOTHING;
					break;
			}
			break;
	}
}

void CSceneContext::OnMouseMove(int xPos, int yPos)
{
	switch (m_nCameraStatus)
	{
		case CAMERA_ORBIT:
			CameraOrbit(m_pfbxScene, m_fbxv4CameraPosition, m_fCameraRoll, xPos-m_xPreviousMousePos, m_yPreviousMousePos-yPos);
			break;
		case CAMERA_ZOOM:
			CameraZoom(m_pfbxScene, m_yPreviousMousePos - yPos, m_nCameraZoomMode);
			m_yPreviousMousePos = yPos;
			break;
		case CAMERA_PAN:
			CameraPan(m_pfbxScene, m_fbxv4CameraPosition, m_fbxv4CameraCenter, m_fCameraRoll, xPos-m_xPreviousMousePos, m_yPreviousMousePos-yPos);
			break;
		default:
			break;
	}
}

void CSceneContext::DisplayGrid(FbxAMatrix& rfbxTransform)
{
	::glPushMatrix();
	::glMultMatrixd(rfbxTransform);

	::glColor3f(0.13f, 0.13f, 0.53f);

	for (int i = -500; i <= 500; i += 20) 
	{
		::glLineWidth(((i % 50) == 0) ? 2.0f : 1.0f);
		if (i == 0) glColor3f(0.83f, 0.13f, 0.13f);
		else glColor3f(0.13f, 0.13f, 0.53f);

		::glBegin(GL_LINES);
		::glVertex3i(i, 0, -500);
		::glVertex3i(i, 0, 500);
		::glEnd();

		::glBegin(GL_LINES);
		::glVertex3i(-500, 0, i);
		::glVertex3i(500, 0, i);
		::glEnd();
	}

	::glPopMatrix();
}


