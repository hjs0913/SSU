/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include <fbxsdk.h>
#include "DisplayCommon.h"

#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void DisplayHierarchy(FbxNode *pfbxNode, int nTabIndents);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
FbxAMatrix GetPoseMatrix2(FbxPose *pfbxPose, int nIndex)
{
	FbxAMatrix fbxPoseMatrix;
	FbxMatrix fbxMatrix = pfbxPose->GetMatrix(nIndex);
	memcpy((double *)fbxPoseMatrix, (double *)fbxMatrix, sizeof(fbxMatrix.mData));

	return(fbxPoseMatrix);
}

FbxAMatrix GetGlobalPosition2(FbxNode *pfbxNode, const FbxTime& pTime, FbxPose *pfbxPose, FbxAMatrix *pfbxmtxParent)
{
	FbxAMatrix fbxmtxGlobal;
	bool bPositionFound = false;

	if (pfbxPose)
	{
		int nNodeIndex = pfbxPose->Find(pfbxNode);
		if (nNodeIndex >= 0)
		{
			if (pfbxPose->IsBindPose() || !pfbxPose->IsLocalMatrix(nNodeIndex))
			{
				fbxmtxGlobal = GetPoseMatrix2(pfbxPose, nNodeIndex);
			}
			else
			{
				FbxAMatrix fbxmtxParent;
				if (pfbxmtxParent)
					fbxmtxParent = *pfbxmtxParent;
				else
				{
					if (pfbxNode->GetParent()) fbxmtxParent = GetGlobalPosition2(pfbxNode->GetParent(), pTime, pfbxPose, NULL);
				}
				FbxAMatrix fbxmtxLocal = GetPoseMatrix2(pfbxPose, nNodeIndex);
				fbxmtxGlobal = fbxmtxParent * fbxmtxLocal; //Column Major Matrix, Right Handed Coordinates
			}
			bPositionFound = true;
		}
	}
	if (!bPositionFound) fbxmtxGlobal = pfbxNode->EvaluateGlobalTransform(pTime);

	return(fbxmtxGlobal);
}

FbxAMatrix GetGeometryOffset(FbxNode *pfbxNode)
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return(FbxAMatrix(T, R, S));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void DisplayControlsPoints(FbxMesh *pfbxMesh, int nControlPoints, int nTabIndents)
{
	DisplayInt("<ControlPoints>: ", nControlPoints, " ", nTabIndents);
	FbxVector4 *pfbxvControlPoints = pfbxMesh->GetControlPoints();
	for (int i = 0; i < nControlPoints; i++) Display3DVector(pfbxvControlPoints[i]);
	DisplayString(" ", "</ControlPoints>", "\n");
}

void DisplayPolygonVertexIndices(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nIndices = 0;
	for (int i = 0; i < nPolygons; i++) nIndices += pfbxMesh->GetPolygonSize(i); //Triangle: 3, Triangulate(), nIndices = nPolygons * 3

	DisplayInt("<Indices>: ", nIndices, " ", nTabIndents);
	for (int i = 0; i < nPolygons; i++)
	{
		int nPolygonSize = pfbxMesh->GetPolygonSize(i); 
		for (int j = 0; j < nPolygonSize; j++) DisplayInt(pfbxMesh->GetPolygonVertex(i, j));
	}
	DisplayString(" ", "</Indices>", "\n");
}

void DisplayPolygonVertexColors(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nElementVertexColors = pfbxMesh->GetElementVertexColorCount();
	if (nElementVertexColors > 0)
	{
		int nColors = 0;
		for (int i = 0; i < nPolygons; i++) nColors += pfbxMesh->GetPolygonSize(i) * nElementVertexColors;

		DisplayInt("<VertexColors>: ", nColors, nElementVertexColors, " ", nTabIndents);
		for (int i = 0, nVertexID = 0; i < nPolygons; i++)
		{
			int nPolygonSize = pfbxMesh->GetPolygonSize(i); //nPolygonSize:3
			for (int j = 0; j < nPolygonSize; j++, nVertexID++)
			{
				int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
				for (int k = 0; k < nElementVertexColors; k++)
				{
					FbxGeometryElementVertexColor *pfbxElementVertexColor = pfbxMesh->GetElementVertexColor(k);
					switch (pfbxElementVertexColor->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
							switch (pfbxElementVertexColor->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									DisplayColor(pfbxElementVertexColor->GetDirectArray().GetAt(nControlPointIndex));
									break;
								case FbxGeometryElement::eIndexToDirect:
									DisplayColor(pfbxElementVertexColor->GetDirectArray().GetAt(pfbxElementVertexColor->GetIndexArray().GetAt(nControlPointIndex)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							switch (pfbxElementVertexColor->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									DisplayColor(pfbxElementVertexColor->GetDirectArray().GetAt(nVertexID));
									break;
								case FbxGeometryElement::eIndexToDirect:
									DisplayColor(pfbxElementVertexColor->GetDirectArray().GetAt(pfbxElementVertexColor->GetIndexArray().GetAt(nVertexID)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygon:
						case FbxGeometryElement::eAllSame:
						case FbxGeometryElement::eNone:
							break;
						default:
							break;
					}
				}
			}
		}
		DisplayString(" ", "</VertexColors>", "\n");
	}
}

void DisplayPolygonVertexUVs(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nElementVertexUVs = pfbxMesh->GetElementUVCount();
	if (nElementVertexUVs > 0)
	{
		int nUVs = 0;
		for (int i = 0; i < nPolygons; i++) nUVs += pfbxMesh->GetPolygonSize(i) * nElementVertexUVs;

		DisplayInt("<VertexUVs>: ", nUVs, nElementVertexUVs, " ", nTabIndents);
		for (int i = 0; i < nPolygons; i++)
		{
			int nPolygonSize = pfbxMesh->GetPolygonSize(i);
			for (int j = 0; j < nPolygonSize; j++)
			{
				int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
				for (int k = 0; k < nElementVertexUVs; k++)
				{
					FbxGeometryElementUV *pfbxElementUV = pfbxMesh->GetElementUV(k);
					switch (pfbxElementUV->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
							switch (pfbxElementUV->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									Display2DVector(pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex));
									break;
								case FbxGeometryElement::eIndexToDirect:
									Display2DVector(pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							{
								int nTextureUVIndex = pfbxMesh->GetTextureUVIndex(i, j);
								switch (pfbxElementUV->GetReferenceMode())
								{
									case FbxGeometryElement::eDirect:
									case FbxGeometryElement::eIndexToDirect:
										Display2DVector(pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex));
										break;
									default:
										break;
								}
							}
							break;
						case FbxGeometryElement::eByPolygon:
						case FbxGeometryElement::eAllSame:
						case FbxGeometryElement::eNone:
							break;
						default:
							break;
					}
				}
			}
		}
		DisplayString(" ", "</VertexUVs>", "\n");
	}
}

void DisplayPolygonVertexNormals(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nElementVertexNormals = pfbxMesh->GetElementNormalCount();
	if (nElementVertexNormals > 0)
	{
		int nNormals = 0;
		for (int i = 0; i < nPolygons; i++) nNormals += pfbxMesh->GetPolygonSize(i) * nElementVertexNormals;

		DisplayInt("<VertexNormals>: ", nNormals, nElementVertexNormals, " ", nTabIndents);
		for (int i = 0, nVertexID = 0; i < nPolygons; i++)
		{
			int nPolygonSize = pfbxMesh->GetPolygonSize(i);
			for (int j = 0; j < nPolygonSize; j++, nVertexID++)
			{
				int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
				for (int k = 0; k < nElementVertexNormals; k++)
				{
					FbxGeometryElementNormal *pfbxElementNormal = pfbxMesh->GetElementNormal(k);
					if (pfbxElementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementNormal->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Display3DVector(pfbxElementNormal->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Display3DVector(pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
		DisplayString(" ", "</VertexNormals>", "\n");
	}
}

void DisplayPolygonVertexTangents(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nElementVertexTangents = pfbxMesh->GetElementTangentCount();
	if (nElementVertexTangents > 0)
	{
		int nTangents = 0;
		for (int i = 0; i < nPolygons; i++) nTangents += pfbxMesh->GetPolygonSize(i) * nElementVertexTangents;

		DisplayInt("<VertexTangents>: ", nTangents, nElementVertexTangents, " ", nTabIndents);
		for (int i = 0, nVertexID = 0; i < nPolygons; i++)
		{
			int nPolygonSize = pfbxMesh->GetPolygonSize(i);
			for (int j = 0; j < nPolygonSize; j++, nVertexID++)
			{
				int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
				for (int k = 0; k < nElementVertexTangents; k++)
				{
					FbxGeometryElementTangent *pfbxElementTangent = pfbxMesh->GetElementTangent(k);
					if (pfbxElementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Display3DVector(pfbxElementTangent->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Display3DVector(pfbxElementTangent->GetDirectArray().GetAt(pfbxElementTangent->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
		DisplayString(" ", "</VertexTangents>", "\n");
	}
}

void DisplayPolygonVertexBinormals(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	int nElementVertexBinormals = pfbxMesh->GetElementTangentCount();
	if (nElementVertexBinormals > 0)
	{
		int nBinormals = 0;
		for (int i = 0; i < nPolygons; i++) nBinormals += pfbxMesh->GetPolygonSize(i) * nElementVertexBinormals;

		DisplayInt("<VertexBinormals>: ", nBinormals, nElementVertexBinormals, " ", nTabIndents);
		for (int i = 0, nVertexID = 0; i < nPolygons; i++)
		{
			int nPolygonSize = pfbxMesh->GetPolygonSize(i);
			for (int j = 0; j < nPolygonSize; j++, nVertexID++)
			{
				for (int k = 0; k < nElementVertexBinormals; k++)
				{
					FbxGeometryElementBinormal *pfbxElementBinormal = pfbxMesh->GetElementBinormal(k);
					if (pfbxElementBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementBinormal->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Display3DVector(pfbxElementBinormal->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Display3DVector(pfbxElementBinormal->GetDirectArray().GetAt(pfbxElementBinormal->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
		DisplayString(" ", "</VertexBinormals>", "\n");
	}
}

void DisplayPolygons(FbxMesh *pfbxMesh, int nPolygons, int nTabIndents)
{
	DisplayInt("<Polygons>: ", nPolygons, "\n", nTabIndents);

	DisplayPolygonVertexIndices(pfbxMesh, nPolygons, nTabIndents + 1);

//	DisplayPolygonVertexColors(pfbxMesh, nPolygons, nTabIndents + 1);
//	DisplayPolygonVertexUVs(pfbxMesh, nPolygons, nTabIndents + 1);
//	DisplayPolygonVertexNormals(pfbxMesh, nPolygons, nTabIndents + 1);
//	DisplayPolygonVertexTangents(pfbxMesh, nPolygons, nTabIndents + 1);
//	DisplayPolygonVertexBinormals(pfbxMesh, nPolygons, nTabIndents + 1);
}

void DisplaySkinDeformers(FbxMesh *pfbxMesh, int nSkins, int nTabIndents)
{
	int nClusters = 0;
	for (int i = 0; i < nSkins; i++) nClusters += ((FbxSkin *)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
	if (nClusters > 0)
	{
		DisplayInt("<SkinDeformers>: ", nSkins, nClusters, "\n", nTabIndents);

		char *pstrClusterModes[] = { "Normalize", "Additive", "Total_1" };
		for (int i = 0; i < nSkins; i++)
		{
			FbxSkin *pfbxSkin = (FbxSkin *)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin);
			int nSkinningType = (int)pfbxSkin->GetSkinningType(); //{ FbxSkin::eRigid, FbxSkin::eLinear, FbxSkin::eDualQuaternion, FbxSkin::eBlend }
			nClusters = pfbxSkin->GetClusterCount();
			DisplayInt("<Skin#> ", i, nClusters, nSkinningType, "\n", nTabIndents + 1);
			for (int j = 0; j < nClusters; j++)
			{
				FbxCluster *pfbxCluster = pfbxSkin->GetCluster(j);
				FbxNode *pfbxClusterLink = pfbxCluster->GetLink();
				if (pfbxClusterLink)
				{
					int nLinkMode = pfbxCluster->GetLinkMode(); //{ "Normalize", "Additive", "Total_1" }
					int nIndices = pfbxCluster->GetControlPointIndicesCount();
					int nAssociateModel = pfbxCluster->GetAssociateModel() ? 1 : 0;
					DisplayInt("<Cluster#>: ", j, nLinkMode, nIndices, nAssociateModel, ReplaceBlank((char *)pfbxClusterLink->GetName(), '_'), nTabIndents + 2);

					int *pnIndices = pfbxCluster->GetControlPointIndices();
					double *pfWeights = pfbxCluster->GetControlPointWeights();
					for (int k = 0; k < nIndices; k++)
					{
						DisplayInt(pnIndices[k]);
						DisplayDouble(pfWeights[k]);
					}

					FbxAMatrix fbxmtxCluster;
					pfbxCluster->GetTransformMatrix(fbxmtxCluster);
					DisplayMatrix(fbxmtxCluster);

					FbxAMatrix fbxmtxGeometryOffset = GetGeometryOffset(pfbxMesh->GetNode());
					DisplayMatrix(fbxmtxGeometryOffset);

					FbxAMatrix fbxmtxClusterLink;
					pfbxCluster->GetTransformLinkMatrix(fbxmtxClusterLink);
					DisplayMatrix(fbxmtxClusterLink);

					//m = fbxmtxCluster * fbxmtxGeometryOffset
					//r = Inverse(fbxmtxClusterLink) * (fbxmtxCluster * fbxmtxGeometryOffset)
					//Inverse() = 

					if (nAssociateModel)
					{
						FbxAMatrix fbxmtxClusterAssociateModel;
						pfbxCluster->GetTransformAssociateModelMatrix(fbxmtxClusterAssociateModel);
						DisplayMatrix(fbxmtxClusterAssociateModel);
					}

					WriteStringToFile("\n");
				}
			}
		}
	}
}

void DisplayHierarchy(FbxScene *pfbxScene)
{
    FbxNode *pfbxRootNode = pfbxScene->GetRootNode();

    for (int i = 0; i < pfbxRootNode->GetChildCount(); i++)
    {
        DisplayHierarchy(pfbxRootNode->GetChild(i), 1);
    }
}

void DisplayHierarchy(FbxNode *pfbxNode, int nTabIndents)
{
	DisplayString("<FrameName>: ", ReplaceBlank(pfbxNode->GetName(), '_'), "\n", nTabIndents);

	int nChilds = pfbxNode->GetChildCount();
	DisplayInt("<Children>: ", nChilds, "\n", nTabIndents + 1);

	FbxNodeAttribute *pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			DisplayString("<Mesh>: ", (char *)pfbxNode->GetName(), "\n", nTabIndents + 1);

			FbxMesh *pfbxMesh = (FbxMesh *)pfbxNode->GetNodeAttribute();

			int nControlPoints = pfbxMesh->GetControlPointsCount();
			if (nControlPoints > 0) DisplayControlsPoints(pfbxMesh, nControlPoints, nTabIndents + 2);

			int nPolygons = pfbxMesh->GetPolygonCount();
			if (nPolygons > 0) DisplayPolygons(pfbxMesh, nPolygons, nTabIndents + 2);

			//DisplayMaterial(pfbxMesh);
			//DisplayTexture(pfbxMesh);

			int nSkins = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nSkins > 0) DisplaySkinDeformers(pfbxMesh, nSkins, nTabIndents + 2);
		}
	}

    for (int i = 0; i < nChilds; i++)
    {
        DisplayHierarchy(pfbxNode->GetChild(i), nTabIndents + 1);
    }
}


