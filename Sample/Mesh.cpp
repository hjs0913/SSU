//-----------------------------------------------------------------------------
// File: CMesh.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMeshIlluminated::CMeshIlluminated(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CMeshIlluminated::~CMeshIlluminated()
{
}

void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, int nVertices)
{
	int nPrimitives = nVertices / 3;
	UINT nIndex0, nIndex1, nIndex2;
	for (int i = 0; i < nPrimitives; i++)
	{
		nIndex0 = i*3+0;
		nIndex1 = i*3+1;
		nIndex2 = i*3+2;
		XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
		XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
		pxmf3Normals[nIndex0] = pxmf3Normals[nIndex1] = pxmf3Normals[nIndex2] = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
	}
}

void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, UINT nVertices, UINT *pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices / 3) : (nVertices / 3);
	XMFLOAT3 xmf3SumOfNormal, xmf3Edge01, xmf3Edge02, xmf3Normal;
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = pnIndices[i*3+0];
			nIndex1 = pnIndices[i*3+1];
			nIndex2 = pnIndices[i*3+2];
			if (pnIndices && ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j)))
			{
				xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, false);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}

void CMeshIlluminated::CalculateTriangleStripVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, UINT nVertices, UINT *pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices - 2) : (nVertices - 2);
	XMFLOAT3 xmf3SumOfNormal(0.0f, 0.0f, 0.0f);
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = ((i % 2) == 0) ? (i + 0) : (i + 1);
			if (pnIndices) nIndex0 = pnIndices[nIndex0];
			nIndex1 = ((i % 2) == 0) ? (i + 1) : (i + 0);
			if (pnIndices) nIndex1 = pnIndices[nIndex1];
			nIndex2 = (pnIndices) ? pnIndices[i + 2] : (i + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}

void CMeshIlluminated::CalculateVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, int nVertices, UINT *pnIndices, int nIndices)
{
	switch (m_d3dPrimitiveTopology)
	{
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			if (pnIndices)
				CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
			else
				CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices);
			break;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
			CalculateTriangleStripVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
			break;
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMeshIlluminatedTextured::CMeshIlluminatedTextured(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
}

CMeshIlluminatedTextured::~CMeshIlluminatedTextured()
{
}

//////////////////////////////////////////////////////////////////////////////////
//
CCubeMeshIlluminatedTextured::CCubeMeshIlluminatedTextured(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMeshIlluminatedTextured(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_nStride = sizeof(CIlluminatedTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	// 사각형 매핑
	XMFLOAT3 pxmf3Positions[36];
	int i = 0;
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);

	XMFLOAT3 pxmf3Normals[36];
	CalculateVertexNormals(pxmf3Normals, pxmf3Positions, m_nVertices, NULL, 0);

	CIlluminatedTexturedVertex pVertices[36];
	for (int i = 0; i < 36; i++) pVertices[i] = CIlluminatedTexturedVertex(pxmf3Positions[i], pxmf3Normals[i]);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CCubeMeshIlluminatedTextured::~CCubeMeshIlluminatedTextured()
{
}

//////////////////////////////////////////////////////////////////////////////////
//
CAirplaneMeshDiffused::CAirplaneMeshDiffused(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmf4Color) : CMeshDiffused(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 192;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	CDiffusedVertex pVertices[192];
	CDiffusedVertex upCenter = CDiffusedVertex(XMFLOAT3(0, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	CDiffusedVertex downCenter = CDiffusedVertex(XMFLOAT3(0, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	float pi = 3.141592f;
	float angle = pi / 8.0f;			// 22.5

	int i = 0;
	// 위 뚜껑 
#pragma region UpShape
	// 1사분면
	pVertices[i++] = upCenter;		// 0, 1, 2
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), +fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 2, 3
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), +fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), +fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		//  0, 3, 4
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), +fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), +fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 4, 5
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), +fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 2사분면
	pVertices[i++] = upCenter;		// 0, 5, 6
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), +fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 6, 7
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), +fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), +fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
		
	pVertices[i++] = upCenter;		// 0, 7, 8
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), +fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), +fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 8, 9
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), +fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 3사분면
	pVertices[i++] = upCenter;		// 0, 9, 10
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), +fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 10, 11
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), +fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), +fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		//  0, 11, 12
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), +fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), +fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 12, 13
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), +fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 4사분면
	pVertices[i++] = upCenter;		// 0, 13, 14
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), +fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 14, 15
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), +fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), +fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 15, 16
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), +fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), +fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = upCenter;		// 0, 16, 1
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), +fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma endregion

	// 옆면
#pragma region SideShape
	// 1사분면
	// 1
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), +fy, fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), -fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), -fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 2
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), +fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), +fy, fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), -fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), +fy, fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), -fy, fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), -fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 3
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), +fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), +fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), -fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), +fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), -fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), -fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 4
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), +fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), +fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), -fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 2사분면
	// 5
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), +fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), -fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), -fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 6
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), +fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), +fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), -fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), +fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), -fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), -fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 7
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), +fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), +fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), -fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), +fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), -fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), -fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 8
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), +fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), +fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), -fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 3사분면
	// 9
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), +fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), -fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), -fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 10
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), +fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), +fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), -fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), +fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), -fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), -fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 11
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), +fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), +fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), -fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), +fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), -fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), -fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 12
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), +fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), +fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), -fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 4사분면
	// 13
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), +fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), -fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, +fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), -fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 14
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), +fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), +fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), -fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), +fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), -fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), -fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 15
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), +fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), +fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), -fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), +fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), -fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), -fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 16
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, +fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), +fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), +fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), -fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma endregion
	
	// 아래  뚜껑 
#pragma region UpShape
	// 1사분면
	pVertices[i++] = downCenter;		// 0, 1, 2
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), -fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 2, 3
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle), -fy, +fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), -fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		//  0, 3, 4
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 2.0f), -fy, fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), -fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 4, 5
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fz * sin(angle * 3.0f), -fy, fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 2사분면
	pVertices[i++] = downCenter;		// 0, 5, 6
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), -fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 6, 7
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle), -fy, -fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), -fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 7, 8
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 2.0f), -fy, -fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), -fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 8, 9
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(fx * cos(angle * 3.0f), -fy, -fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 3사분면
	pVertices[i++] = downCenter;		// 0, 9, 10
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0, -fy, -fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), -fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 10, 11
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle), -fy, -fz * cos(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), -fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		//  0, 11, 12
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 2.0f), -fy, -fz * cos(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), -fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 12, 13
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fz * sin(angle * 3.0f), -fy, -fz * cos(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	// 4사분면
	pVertices[i++] = downCenter;		// 0, 13, 14
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -fy, 0), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), -fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 14, 15
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle), -fy, fx * sin(angle)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), -fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 15, 16
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 2.0f), -fy, fx * sin(angle * 2.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), -fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));

	pVertices[i++] = downCenter;		// 0, 16, 1
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx * cos(angle * 3.0f), -fy, fx * sin(angle * 3.0f)), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	pVertices[i] = CDiffusedVertex(XMFLOAT3(0, -fy, +fz), Vector4::Add(xmf4Color, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma endregion

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CAirplaneMeshDiffused::~CAirplaneMeshDiffused()
{
}

