/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _SCENE_CONTEXT_H
#define _SCENE_CONTEXT_H

#include "GlFunctions.h"

#define WINDOW_WIDTH				800
#define WINDOW_HEIGHT				600

#define LEFT_BUTTON					0
#define MIDDLE_BUTTON				1
#define RIGHT_BUTTON				2

#define BUTTON_DOWN					0
#define BUTTON_UP					1

#define CAMERA_NOTHING				0
#define CAMERA_ORBIT				1
#define CAMERA_ZOOM					2
#define CAMERA_PAN					3

#define ZOOM_FOCAL_LENGTH			0
#define ZOOM_POSITION				1

#define OBJECTS						1

class CSceneContext
{
public:
    CSceneContext();
    ~CSceneContext();

    bool OnDisplay();
    void OnMouseClick(int nButton, int nState, int xPos, int yPos);
    void OnMouseMove(int xPos, int yPos);
    void OnTimer();

    bool SetCurrentAnimStack(int nIndex);

	void DisplayGrid(FbxAMatrix& rfbxTransform);

public:
	unsigned int GetFrameTimeByMilliSeconds() { return((unsigned int)m_fbxFrameTime.GetMilliSeconds()); }

    FbxManager *m_pfbxSdkManager = NULL;
    FbxScene *m_pfbxScene = NULL;
    FbxImporter *m_pfbxImporter = NULL;
    FbxAnimLayer *m_pfbxCurrentAnimLayer = NULL;

    FbxArray<FbxString *> m_fbxAnimationStackNameArray;

	FbxTime m_pfbxStartTimes[OBJECTS];
	FbxTime m_pfbxStopTimes[OBJECTS];
	FbxTime m_pfbxCurrentTimes[OBJECTS];
	FbxTime m_fbxFrameTime;

	int m_xPreviousMousePos;
	int m_yPreviousMousePos;
	FbxVector4 m_fbxv4CameraPosition;
	FbxVector4 m_fbxv4CameraCenter;
    double m_fCameraRoll;
    int m_nCameraStatus = CAMERA_ORBIT;
    int m_nCameraZoomMode = ZOOM_FOCAL_LENGTH;
};

#endif 
