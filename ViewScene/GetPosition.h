/****************************************************************************************

Copyright (C) 2015 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/
 
#ifndef _GET_POSITION_H
#define _GET_POSITION_H
 
#include <fbxsdk.h>

FbxAMatrix GetGlobalTransform(FbxNode *pfbxNode, const FbxTime& rfbxTime, FbxPose *pfbxPose = NULL, FbxAMatrix *fbxmtxParentGlobalTransform = NULL);
FbxAMatrix GetPoseMatrix(FbxPose *pfbxPose, int nNodeIndex);
FbxAMatrix GetToNodeTransform(FbxNode *pfbxNode);

#endif // #ifndef _GET_POSITION_H



