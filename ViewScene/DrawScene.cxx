#include "GlFunctions.h"

void MatrixScale(FbxAMatrix& fbxmtxSrcMatrix, double pValue)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) fbxmtxSrcMatrix[i][j] *= pValue;
	}
}

void MatrixAddToDiagonal(FbxAMatrix& fbxmtxSrcMatrix, double pValue)
{
	fbxmtxSrcMatrix[0][0] += pValue;
	fbxmtxSrcMatrix[1][1] += pValue;
	fbxmtxSrcMatrix[2][2] += pValue;
	fbxmtxSrcMatrix[3][3] += pValue;
}

void MatrixAdd(FbxAMatrix& fbxmtxDstMatrix, FbxAMatrix& fbxmtxSrcMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) fbxmtxDstMatrix[i][j] += fbxmtxSrcMatrix[i][j];
	}
}

FbxAMatrix GetGeometricTransform(FbxNode *pfbxNode)
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return(FbxAMatrix(T, R, S));
}

FbxAMatrix ComputeClusterDeformation(FbxMesh *pfbxMesh, FbxCluster *pfbxCluster, FbxTime& rfbxTime, FbxCluster::ELinkMode nClusterMode)
{
	FbxAMatrix fbxmtxVertexTransform;

//    FbxCluster::ELinkMode nClusterMode = pfbxCluster->GetLinkMode();
	if (nClusterMode == FbxCluster::eNormalize) 
	{
		FbxAMatrix fbxmtxGeometryOffset = GetGeometricTransform(pfbxMesh->GetNode());

		FbxAMatrix fbxmtxBindPoseMeshToRoot; //Cluster Transform
		pfbxCluster->GetTransformMatrix(fbxmtxBindPoseMeshToRoot);

		FbxAMatrix fbxmtxBindPoseBoneToRoot; //Cluster Link Transform
		pfbxCluster->GetTransformLinkMatrix(fbxmtxBindPoseBoneToRoot);

		FbxAMatrix fbxmtxAnimatedBoneToRoot = pfbxCluster->GetLink()->EvaluateGlobalTransform(rfbxTime); //Cluster Link Node Global Transform

		fbxmtxVertexTransform = fbxmtxAnimatedBoneToRoot * fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot * fbxmtxGeometryOffset;
	}
	else
	{ //FbxCluster::eAdditive
		if (pfbxCluster->GetAssociateModel())
		{
			FbxAMatrix fbxmtxAssociateModel;
			pfbxCluster->GetTransformAssociateModelMatrix(fbxmtxAssociateModel);

			FbxAMatrix fbxmtxAssociateGeometryOffset = GetGeometricTransform(pfbxCluster->GetAssociateModel());
			fbxmtxAssociateModel *= fbxmtxAssociateGeometryOffset;

			FbxAMatrix fbxmtxAssociateModelGlobal = pfbxCluster->GetAssociateModel()->EvaluateGlobalTransform(rfbxTime);

			FbxAMatrix fbxmtxClusterTransform;
			pfbxCluster->GetTransformMatrix(fbxmtxClusterTransform);

			FbxAMatrix fbxmtxGeometryOffset = GetGeometricTransform(pfbxMesh->GetNode());
			fbxmtxClusterTransform *= fbxmtxGeometryOffset;

			FbxAMatrix fbxmtxClusterLinkTransform;
			pfbxCluster->GetTransformLinkMatrix(fbxmtxClusterLinkTransform);

			FbxAMatrix fbxmtxLinkGeometryOffset = GetGeometricTransform(pfbxCluster->GetLink());
			fbxmtxClusterLinkTransform *= fbxmtxLinkGeometryOffset;

			FbxAMatrix fbxmtxClusterLinkToRoot = pfbxCluster->GetLink()->EvaluateGlobalTransform(rfbxTime);

			fbxmtxVertexTransform = fbxmtxClusterTransform.Inverse() * fbxmtxAssociateModel * fbxmtxAssociateModelGlobal.Inverse() * fbxmtxClusterLinkToRoot * fbxmtxClusterLinkTransform.Inverse() * fbxmtxClusterTransform;
		}
	}
	return(fbxmtxVertexTransform);
}

void ComputeLinearDeformation(FbxMesh *pfbxMesh, FbxTime& rfbxTime, FbxVector4 *pfbxv4Vertices, int nVertices)
{
	FbxAMatrix *pfbxmtxClusterDeformations = new FbxAMatrix[nVertices];
	::memset(pfbxmtxClusterDeformations, 0, nVertices * sizeof(FbxAMatrix));

	double *pfSumOfClusterWeights = new double[nVertices];
	::memset(pfSumOfClusterWeights, 0, nVertices * sizeof(double));

	FbxCluster::ELinkMode nClusterMode = ((FbxSkin *)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();
	if (nClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < nVertices; ++i) pfbxmtxClusterDeformations[i].SetIdentity();
	}

	int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	for ( int i = 0; i < nSkinDeformers; i++)
	{
		FbxSkin *pfbxSkinDeformer = (FbxSkin *)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin);	
		int nClusters = pfbxSkinDeformer->GetClusterCount(); //Bones which influences vertex deformation

		int nSkinControlPointIndices = pfbxSkinDeformer->GetControlPointIndicesCount();
		int *pfSkinControlPointIndices = pfbxSkinDeformer->GetControlPointIndices();
		double *pfSkinBlendWeights = pfbxSkinDeformer->GetControlPointBlendWeights();
		
		for (int j = 0; j < nClusters; j++)
		{
			FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);
			if (!pfbxCluster->GetLink()) continue;

			FbxAMatrix fbxmtxClusterDeformation = ComputeClusterDeformation(pfbxMesh, pfbxCluster, rfbxTime, nClusterMode);

			int *pnIndices = pfbxCluster->GetControlPointIndices();
			double *pfWeights = pfbxCluster->GetControlPointWeights();

			int nIndices = pfbxCluster->GetControlPointIndicesCount();
			for (int k = 0; k < nIndices; k++)
			{            
				int nVertex = pnIndices[k];
				double fWeight = pfWeights[k];
				if ((nVertex >= nVertices) || (fWeight == 0.0)) continue;

				FbxAMatrix fbxmtxInfluence = fbxmtxClusterDeformation;
				MatrixScale(fbxmtxInfluence, fWeight);

				if (nClusterMode == FbxCluster::eAdditive)
				{    
					MatrixAddToDiagonal(fbxmtxInfluence, 1.0 - fWeight);
					pfbxmtxClusterDeformations[nVertex] = fbxmtxInfluence * pfbxmtxClusterDeformations[nVertex];
					pfSumOfClusterWeights[nVertex] = 1.0;
				}
				else 
				{
					MatrixAdd(pfbxmtxClusterDeformations[nVertex], fbxmtxInfluence);
					pfSumOfClusterWeights[nVertex] += fWeight;
				}
			}			
		}
	}

	for (int i = 0; i < nVertices; i++) 
	{
		if (pfSumOfClusterWeights[i] != 0.0)
		{
			FbxVector4 fbxv4Vertex = pfbxv4Vertices[i];
			pfbxv4Vertices[i] = pfbxmtxClusterDeformations[i].MultT(fbxv4Vertex);
			if (nClusterMode == FbxCluster::eNormalize)
			{
//				pfbxv4Vertices[i] /= pfSumOfClusterWeights[i];
			}
			else if (nClusterMode == FbxCluster::eTotalOne)
			{
				fbxv4Vertex *= (1.0 - pfSumOfClusterWeights[i]);
				pfbxv4Vertices[i] += fbxv4Vertex;
			}
		} 
	}

	delete[] pfbxmtxClusterDeformations;
	delete[] pfSumOfClusterWeights;
}

void ComputeDualQuaternionDeformation(FbxMesh *pfbxMesh, FbxTime& rfbxTime, FbxVector4 *pfbxv4Vertices, int nVertices)
{
	FbxDualQuaternion *pfbxDQClusterDeformations = new FbxDualQuaternion[nVertices];
	memset(pfbxDQClusterDeformations, 0, nVertices * sizeof(FbxDualQuaternion));
	double *pfClusterWeights = new double[nVertices];
	memset(pfClusterWeights, 0, nVertices * sizeof(double));

	FbxCluster::ELinkMode nClusterMode = ((FbxSkin *)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();
	int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int i = 0; i < nSkinDeformers; i++)
	{
		FbxSkin *pfbxSkinDeformer = (FbxSkin *)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin);
		int nClusters = pfbxSkinDeformer->GetClusterCount();
		for (int j = 0; j < nClusters; j++)
		{
			FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);
			if (!pfbxCluster->GetLink()) continue;

			FbxAMatrix fbxmtxCluster = ComputeClusterDeformation(pfbxMesh, pfbxCluster, rfbxTime, nClusterMode);

			FbxQuaternion Q = fbxmtxCluster.GetQ();
			FbxVector4 T = fbxmtxCluster.GetT();
			FbxDualQuaternion fbxDualQuaternion(Q, T);

			int nIndices = pfbxCluster->GetControlPointIndicesCount();
			int *pnControlPointIndices = pfbxCluster->GetControlPointIndices();
			double *pfControlPointWeights = pfbxCluster->GetControlPointWeights();
			for (int k = 0; k < nIndices; ++k) 
			{ 
				int nIndex = pnControlPointIndices[k];
				if (nIndex >= nVertices) continue;

				double fWeight = pfControlPointWeights[k];
				if (fWeight == 0.0) continue;

				FbxDualQuaternion fbxmtxInfluence = fbxDualQuaternion * fWeight;
				if (nClusterMode == FbxCluster::eAdditive)
				{    
					pfbxDQClusterDeformations[nIndex] = fbxmtxInfluence;
					pfClusterWeights[nIndex] = 1.0;
				}
				else // FbxCluster::eNormalize || FbxCluster::eTotalOne
				{
					if (j == 0)
					{
						pfbxDQClusterDeformations[nIndex] = fbxmtxInfluence;
					}
					else
					{
						double fSign = pfbxDQClusterDeformations[nIndex].GetFirstQuaternion().DotProduct(fbxDualQuaternion.GetFirstQuaternion());
						if (fSign >= 0.0 )
						{
							pfbxDQClusterDeformations[nIndex] += fbxmtxInfluence;
						}
						else
						{
							pfbxDQClusterDeformations[nIndex] -= fbxmtxInfluence;
						}
					}
					pfClusterWeights[nIndex] += fWeight;
				}
			}
		}
	}

	for (int i = 0; i < nVertices; i++) 
	{
		FbxVector4 fbxv4SrcVertex = pfbxv4Vertices[i];
		double fWeightSum = pfClusterWeights[i];

		if (fWeightSum != 0.0) 
		{
			pfbxDQClusterDeformations[i].Normalize();
			pfbxv4Vertices[i] = pfbxDQClusterDeformations[i].Deform(pfbxv4Vertices[i]);

			if (nClusterMode == FbxCluster::eNormalize)
			{
				pfbxv4Vertices[i] /= fWeightSum;
			}
			else if (nClusterMode == FbxCluster::eTotalOne)
			{
				fbxv4SrcVertex *= (1.0 - fWeightSum);
				pfbxv4Vertices[i] += fbxv4SrcVertex;
			}
		} 
	}

	delete[] pfbxDQClusterDeformations;
	delete[] pfClusterWeights;
}

void ComputeSkinDeformation(FbxMesh *pfbxMesh, FbxTime& rfbxTime, FbxVector4 *pfbxv4Vertices, int nVertices)
{
	FbxSkin *pfbxSkinDeformer = (FbxSkin *)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType nSkinningType = pfbxSkinDeformer->GetSkinningType();

	if ((nSkinningType == FbxSkin::eLinear) || (nSkinningType == FbxSkin::eRigid))
	{
		ComputeLinearDeformation(pfbxMesh, rfbxTime, pfbxv4Vertices, nVertices);
	}
	else if (nSkinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(pfbxMesh, rfbxTime, pfbxv4Vertices, nVertices);
	}
	else if (nSkinningType == FbxSkin::eBlend)
	{
		FbxVector4 *pfbxv4LinearVertices = new FbxVector4[nVertices];
		memcpy(pfbxv4LinearVertices, pfbxMesh->GetControlPoints(), nVertices * sizeof(FbxVector4));
		ComputeLinearDeformation(pfbxMesh, rfbxTime, pfbxv4LinearVertices, nVertices);

		FbxVector4 *pfbxv4DQVertices = new FbxVector4[nVertices];
		memcpy(pfbxv4DQVertices, pfbxMesh->GetControlPoints(), nVertices * sizeof(FbxVector4));
		ComputeDualQuaternionDeformation(pfbxMesh, rfbxTime, pfbxv4DQVertices, nVertices);

		int nBlendWeights = pfbxSkinDeformer->GetControlPointIndicesCount();
		double *pfControlPointBlendWeights = pfbxSkinDeformer->GetControlPointBlendWeights();
		for (int i = 0; i < nBlendWeights; i++)
		{
			pfbxv4Vertices[i] = pfbxv4DQVertices[i] * pfControlPointBlendWeights[i] + pfbxv4LinearVertices[i] * (1 - pfControlPointBlendWeights[i]);
		}

		delete[] pfbxv4LinearVertices;
		delete[] pfbxv4DQVertices;
	}
}

void RenderFbxMesh(FbxMesh *pfbxMesh, FbxAMatrix& fbxmtxNodeToRoot, FbxAMatrix& fbxmtxGeometryOffset, FbxAMatrix fbxmtxWorld)
{
	int nVertices = pfbxMesh->GetControlPointsCount();
	if (nVertices > 0)
	{
		FbxAMatrix fbxmtxTransform = fbxmtxWorld;
		int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		if (nSkinDeformers == 0) fbxmtxTransform = fbxmtxWorld * fbxmtxNodeToRoot * fbxmtxGeometryOffset;

		FbxVector4 *pfbxv4Vertices = (FbxVector4 *)pfbxMesh->GetUserDataPtr();

		::glPushMatrix();
		::glMultMatrixd((const double *)fbxmtxTransform);

		if (nSkinDeformers > 0) ::glColor4f(0.75f, 0.3f, 0.2f, 1.0f);
		else ::glColor4f(0.2f, 0.3f, 0.75f, 1.0f);
		::glLineWidth(1.0f);

		int nPolygons = pfbxMesh->GetPolygonCount();
		for (int i = 0; i < nPolygons; i++)
		{
			int nVerticesPerPolygon = pfbxMesh->GetPolygonSize(i);
			::glBegin(GL_LINE_LOOP);
			for (int j = 0; j < nVerticesPerPolygon; j++)
			{
				::glVertex3dv((GLdouble *)pfbxv4Vertices[pfbxMesh->GetPolygonVertex(i, j)]);
			}
			::glEnd();
		}

		::glPopMatrix();
	}
}

void RenderFbxNodeHierarchy(FbxNode *pfbxNode, FbxTime& fbxCurrentTime, FbxAMatrix& fbxmtxWorld)
{
	FbxNodeAttribute *pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute && (pfbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh))
	{
		FbxAMatrix fbxmtxNodeToRoot = pfbxNode->EvaluateGlobalTransform(fbxCurrentTime);
		FbxAMatrix fbxmtxGeometryOffset = GetGeometricTransform(pfbxNode);

		FbxMesh *pfbxMesh = pfbxNode->GetMesh();
		RenderFbxMesh(pfbxMesh, fbxmtxNodeToRoot, fbxmtxGeometryOffset, fbxmtxWorld);
	}

	int nChilds = pfbxNode->GetChildCount();
	for (int i = 0; i < nChilds; i++) RenderFbxNodeHierarchy(pfbxNode->GetChild(i), fbxCurrentTime, fbxmtxWorld);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void AnimateFbxMesh(FbxMesh *pfbxMesh, FbxTime& fbxCurrentTime)
{
	int nVertices = pfbxMesh->GetControlPointsCount();
	if (nVertices > 0)
	{
		FbxVector4 *pfbxv4Vertices = new FbxVector4[nVertices];
		::memcpy(pfbxv4Vertices, pfbxMesh->GetControlPoints(), nVertices * sizeof(FbxVector4));

		int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		if (nSkinDeformers > 0) ComputeSkinDeformation(pfbxMesh, fbxCurrentTime, pfbxv4Vertices, nVertices);
		//{
		//	int nClusters = 0;
		//	for (int i = 0; i < nSkinDeformers; ++i) nClusters += ((FbxSkin *)(pfbxMesh->GetDeformer(i, FbxDeformer::eSkin)))->GetClusterCount();
		//	if (nClusters > 0) ComputeSkinDeformation(pfbxMesh, fbxCurrentTime, pfbxv4Vertices, nVertices);
		//}

		pfbxMesh->SetUserDataPtr(pfbxv4Vertices);
	}
}

void AnimateFbxNodeHierarchy(FbxNode *pfbxNode, FbxTime& fbxCurrentTime)
{
	FbxNodeAttribute *pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute && (pfbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh))
	{
		FbxMesh *pfbxMesh = pfbxNode->GetMesh();
		AnimateFbxMesh(pfbxMesh, fbxCurrentTime);
	}

	int nChilds = pfbxNode->GetChildCount();
	for (int i = 0; i < nChilds; i++) AnimateFbxNodeHierarchy(pfbxNode->GetChild(i), fbxCurrentTime);
}

