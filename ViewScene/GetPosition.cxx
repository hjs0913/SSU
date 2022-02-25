/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This file contains the functions to get the global 
// position of a node for a given time in the current animation stack.
//
/////////////////////////////////////////////////////////////////////////

#include "GetPosition.h"

FbxAMatrix GetPoseMatrix(FbxPose *pfbxPose, int nIndex)
{
	FbxAMatrix fbxmtxPose;
	FbxMatrix fbxMatrix = pfbxPose->GetMatrix(nIndex);
	memcpy((double *)fbxmtxPose, (double *)fbxMatrix, sizeof(fbxMatrix.mData));

	return(fbxmtxPose);
}

FbxAMatrix GetGlobalTransform(FbxNode *pfbxNode, const FbxTime& rfbxTime, FbxPose *pfbxPose, FbxAMatrix *pfbxmtxParent)
{
	FbxAMatrix fbxmtxGlobal;
	bool bFound = false;

	if (pfbxPose)
	{
		int nNodeIndex = pfbxPose->Find(pfbxNode);
		if (nNodeIndex >= 0)
		{
			if (pfbxPose->IsBindPose() || !pfbxPose->IsLocalMatrix(nNodeIndex))
			{
				fbxmtxGlobal = GetPoseMatrix(pfbxPose, nNodeIndex);
			}
			else
			{
				FbxAMatrix fbxmtxParent;
				if (pfbxmtxParent)
					fbxmtxParent = *pfbxmtxParent;
				else
				{
					if (pfbxNode->GetParent()) fbxmtxParent = GetGlobalTransform(pfbxNode->GetParent(), rfbxTime, pfbxPose, NULL);
				}
				FbxAMatrix fbxmtxLocal = GetPoseMatrix(pfbxPose, nNodeIndex);
				fbxmtxGlobal = fbxmtxParent * fbxmtxLocal; //Column Major Matrix, Right Handed Coordinates
			}
			bFound = true;
		}
	}
	if (!bFound) fbxmtxGlobal = pfbxNode->EvaluateGlobalTransform(rfbxTime);

	return(fbxmtxGlobal);
}

FbxAMatrix GetToNodeTransform(FbxNode *pfbxNode)
{
    const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return(FbxAMatrix(T, R, S));
}

