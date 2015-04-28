#include "stdafx.h"
#include "cMousePicking.h"
#include "cTerrain.h"
#include "cQuadTree.h"
#include "cOctree.h"

#define MAP_SIZE 257

cMousePicking::cMousePicking()
	: m_pVertexBuffer(NULL)
	, m_pIndexBuffer(NULL)
	, m_pQuadTree(NULL)
	, m_pOctree(NULL)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_matMeshWorld, I);

	I = XMMatrixIdentity();
	I = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	XMStoreFloat4x4(&m_matScale, I);

	m_vPickingPoint = { 0.f, 0.f, 0.f };

	m_nPickedTriangle = -1;

	m_fRho = 50.f;
	m_eEditType = E_EMPTY;

    m_eKeyTest = E_UP;
    m_eKeyReturn = E_UP;

	m_eTextureType = DirectX::E_ALPHAEMPTY;
	m_isRightClick = false;
}

cMousePicking::~cMousePicking()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_DELETE(m_pQuadTree);
	SAFE_DELETE(m_pOctree);
}

void cMousePicking::Setup()
{
	m_mtMesh.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_mtMesh.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_mtMesh.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	m_mtPickedTriangle.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	m_mtPickedTriangle.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	m_mtPickedTriangle.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

void cMousePicking::Init(ID3D11Buffer* pVertexBuffer
	, ID3D11Buffer* pIndexBuffer
	, std::vector<float> vecHeight)
{
	BuildMeshGeometryBuffers();
	m_vecHeight = vecHeight;

	if (m_vecVertex.size() == 0)
	{
		m_vecVertex.clear();
		m_vecVertex.reserve(m_vecHeight.size());

		for (int i = 0; i < m_vecHeight.size(); i++)
		{
			Vertex::ST_P_VERTEX vertex;
			vertex.Pos.x = (int)(i % MAP_SIZE);
			vertex.Pos.z = 256 - (int)(i / 257);
			vertex.Pos.y = m_vecHeight[i];
			m_vecVertex.push_back(vertex);
		}
	}
}

void cMousePicking::KeyUpdate(bool isUpdate)
{
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (m_eKeyTest == E_UP)
        {
            m_eKeyTest = E_KEYDOWN;
        }
        else if (m_eKeyTest == E_KEYDOWN)
        {
            m_eKeyTest = E_DOWN;
        }
    }
    else
    {
        if (m_eKeyTest == E_DOWN)
        {
            m_eKeyTest = E_KEYUP;
        }
        else if (m_eKeyTest == E_KEYUP)
        {
            m_eKeyTest = E_UP;
        }
    }

    if (GetAsyncKeyState('P') & 0x8000)
    {
        if (m_eKeyReturn == E_UP)
        {
            m_eKeyReturn = E_KEYDOWN;
        }
        else if (m_eKeyReturn == E_KEYDOWN)
        {
            m_eKeyReturn = E_DOWN;
        }
    }
    else
    {
        if (m_eKeyReturn == E_DOWN)
        {
            m_eKeyReturn = E_KEYUP;
        }
        else if (m_eKeyReturn == E_KEYUP)
        {
            m_eKeyReturn = E_UP;
        }
    }
}

void cMousePicking::Update(float fDelta)
{
    KeyUpdate(true);
    HeightEdit();
}

void cMousePicking::Render(DirectionalLight lights[3])
{
	std::wostringstream outs;
	outs.precision(3);
	outs << L"X = " << m_vPickingPoint.x << L"   Z = " << m_vPickingPoint.z
		<< L"   Y = " << m_vPickingPoint.y;

	SetWindowText(g_hWnd, outs.str().c_str());

	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::Basic32);
	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = g_pCamera->View();
	XMMATRIX proj = g_pCamera->Proj();
	XMMATRIX viewProj = g_pCamera->ViewProj();

	Effects::BasicFX->SetEyePosW(g_pCamera->GetPosition());
	Effects::BasicFX->SetDirLights(lights);

	ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeMeshTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the Mesh.

		if (GetAsyncKeyState('1') & 0x8000)
			g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::WireframeRS);

		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMFLOAT4X4 matWorld;
		XMMATRIX I = XMMatrixIdentity();
		I = XMMatrixTranslation(m_vPickingPoint.x, m_vPickingPoint.y, m_vPickingPoint.z);
		
		
		XMMATRIX scale = XMLoadFloat4x4(&m_matScale);
		I = scale * I;
		
		XMStoreFloat4x4(&matWorld, I);

		XMMATRIX world = XMLoadFloat4x4(&matWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);

		//////////
		Effects::BasicFX->SetMaterial(m_mtMesh);
		activeMeshTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);
		g_pD3DDevice->m_pDevCon->DrawIndexed(m_nMeshIndexCount, 0, 0);

		// Restore default
		g_pD3DDevice->m_pDevCon->RSSetState(0);
		//////////
	}
}

void cMousePicking::OnMouseDown(WPARAM btnState, int nX, int nY)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		m_isRightClick = true;
		Pick(nX, nY);
		SelectCircle(m_vPickingPoint.x, m_vPickingPoint.z, 5);
		SetMappingData();
    }
	else
	{
		m_isRightClick = false;
	}
}


void cMousePicking::BuildMeshGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	m_vecMeshVertices.resize(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> m_vecMeshVertices[i].Pos.x >> m_vecMeshVertices[i].Pos.y >> m_vecMeshVertices[i].Pos.z;
		fin >> m_vecMeshVertices[i].Normal.x >> m_vecMeshVertices[i].Normal.y >> m_vecMeshVertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&m_vecMeshVertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mMeshBox.Center, 0.5f*(vMin + vMax));
	XMStoreFloat3(&mMeshBox.Extents, 0.5f*(vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_nMeshIndexCount = 3 * tcount;
	m_vecMeshIndices.resize(m_nMeshIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> m_vecMeshIndices[i * 3 + 0] >> m_vecMeshIndices[i * 3 + 1] >> m_vecMeshIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &m_vecMeshVertices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_nMeshIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &m_vecMeshIndices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer));
}

void cMousePicking::Pick(int nX, int nY)
{
	XMMATRIX P = g_pCamera->Proj();

	float vx = (+2.0f*nX / g_pD3DDevice->m_nClientWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f*nY / g_pD3DDevice->m_nClientHeight + 1.0f) / P(1, 1);

	XMVECTOR rayOrigin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	XMMATRIX V = g_pCamera->View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = XMMatrixIdentity();
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	rayDir = XMVector3Normalize(rayDir);


	m_pOctree = new cOctree(MAP_SIZE);
	CalPoint(m_pOctree, rayOrigin, rayDir, 0);
	SAFE_DELETE(m_pOctree);
}

bool cMousePicking::CalTail(cQuadTree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist)
{

	bool isColliedTri = false;

	if (pRoot->AddChild())
	{
		std::vector<ST_INDEX> vecIndex;
		vecIndex.resize(4);
		
		for (int i = 0; i < pRoot->m_vecChild.size(); i++)
		{
			vecIndex[i].i0 = pRoot->m_vecChild[i]->GetIndex()[0];
			vecIndex[i].i1 = pRoot->m_vecChild[i]->GetIndex()[1];
			vecIndex[i].i2 = pRoot->m_vecChild[i]->GetIndex()[2];
			vecIndex[i].i3 = pRoot->m_vecChild[i]->GetIndex()[3];

			vecIndex[i].v0 = XMLoadFloat3(&m_vecVertex[vecIndex[i].i0].Pos);
			vecIndex[i].v1 = XMLoadFloat3(&m_vecVertex[vecIndex[i].i1].Pos);
			vecIndex[i].v2 = XMLoadFloat3(&m_vecVertex[vecIndex[i].i2].Pos);
			vecIndex[i].v3 = XMLoadFloat3(&m_vecVertex[vecIndex[i].i3].Pos);

			isColliedTri = XNA::IntersectRayTriangle(vOrigin, vDir,
				vecIndex[i].v0, vecIndex[i].v2, vecIndex[i].v3, &fDist);

			if (isColliedTri)
			{
				pRoot = pRoot->m_vecChild[i];

				m_vecQuad.push_back(pRoot);

				XMVECTOR vPoint;
				vPoint = vOrigin + (fDist * vDir);
				//XMStoreFloat3(&m_vPickingPoint, vPoint);
				m_vecTest.push_back(vPoint);
				CalTail(pRoot, vOrigin, vDir, fDist);
				return false;

			}

			isColliedTri = XNA::IntersectRayTriangle(vOrigin, vDir,
				vecIndex[i].v3, vecIndex[i].v1, vecIndex[i].v0, &fDist);

			if (isColliedTri)
			{
				pRoot = pRoot->m_vecChild[i];

				m_vecQuad.push_back(pRoot);

				XMVECTOR vPoint;
				vPoint = vOrigin + (fDist * vDir);
				//XMStoreFloat3(&m_vPickingPoint, vPoint);
				m_vecTest.push_back(vPoint);
				CalTail(pRoot, vOrigin, vDir, fDist);
				return false;
			}



		}
	}
	else
	{
		XMVECTOR vPoint;
		vPoint = vOrigin + (fDist * vDir);
		XMStoreFloat3(&m_vPickingPoint, vPoint);
		m_vecTest.clear();
		return true;
	}
}

XMFLOAT3 cMousePicking::SetIndex(int nIndex, int nSize)
{
	XMFLOAT3 vPos;
	
	int nValue = nIndex / (nSize*nSize);
	int nMod = nIndex % (nSize*nSize);

	vPos.x = nMod % nSize;
	vPos.y = nValue;
	vPos.z = nMod / nSize;

	return vPos;
}

void cMousePicking::CalPoint(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist)
{
	bool isColliedTri = false;

	if (pRoot->AddChild())
	{
		ST_OCTREE_INDEX OctreeIndex;

		for (int i = 0; i < pRoot->GetChild().size(); i++)
		{
			bool isColliedTri0 = false;
			bool isColliedTri1 = false;
			bool isColliedTri2 = false;
			bool isColliedTri3 = false;
			bool isColliedTri4 = false;
			bool isColliedTri5 = false;
			bool isColliedTri6 = false;
			bool isColliedTri7 = false;
			bool isColliedTri8 = false;
			bool isColliedTri9 = false;
			bool isColliedTri10 = false;
			bool isColliedTri11 = false;

			OctreeIndex.i0 = pRoot->GetChild()[i]->GetIndex()[0];
			OctreeIndex.i1 = pRoot->GetChild()[i]->GetIndex()[1];
			OctreeIndex.i2 = pRoot->GetChild()[i]->GetIndex()[2];
			OctreeIndex.i3 = pRoot->GetChild()[i]->GetIndex()[3];
			OctreeIndex.i4 = pRoot->GetChild()[i]->GetIndex()[4];
			OctreeIndex.i5 = pRoot->GetChild()[i]->GetIndex()[5];
			OctreeIndex.i6 = pRoot->GetChild()[i]->GetIndex()[6];
			OctreeIndex.i7 = pRoot->GetChild()[i]->GetIndex()[7];

			OctreeIndex.v0 = XMLoadFloat3(&SetIndex(OctreeIndex.i0, MAP_SIZE));
			OctreeIndex.v1 = XMLoadFloat3(&SetIndex(OctreeIndex.i1, MAP_SIZE));
			OctreeIndex.v2 = XMLoadFloat3(&SetIndex(OctreeIndex.i2, MAP_SIZE));
			OctreeIndex.v3 = XMLoadFloat3(&SetIndex(OctreeIndex.i3, MAP_SIZE));
			OctreeIndex.v4 = XMLoadFloat3(&SetIndex(OctreeIndex.i4, MAP_SIZE));
			OctreeIndex.v5 = XMLoadFloat3(&SetIndex(OctreeIndex.i5, MAP_SIZE));
			OctreeIndex.v6 = XMLoadFloat3(&SetIndex(OctreeIndex.i6, MAP_SIZE));
			OctreeIndex.v7 = XMLoadFloat3(&SetIndex(OctreeIndex.i7, MAP_SIZE));

			isColliedTri0 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v4, OctreeIndex.v5, OctreeIndex.v1, &fDist);
			if (isColliedTri0)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri1 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v1, OctreeIndex.v0, OctreeIndex.v4, &fDist);
			if (isColliedTri1)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri2 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v5, OctreeIndex.v7, OctreeIndex.v3, &fDist);
			if (isColliedTri2)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri3 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v3, OctreeIndex.v1, OctreeIndex.v5, &fDist);
			if (isColliedTri3)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri4 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v7, OctreeIndex.v3, OctreeIndex.v2, &fDist);
			if (isColliedTri4)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri5 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v2, OctreeIndex.v6, OctreeIndex.v7, &fDist);
			if (isColliedTri5)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri6 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v6, OctreeIndex.v4, OctreeIndex.v0, &fDist);
			if (isColliedTri6)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri7 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v0, OctreeIndex.v2, OctreeIndex.v6, &fDist);
			if (isColliedTri7)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri8 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v6, OctreeIndex.v7, OctreeIndex.v5, &fDist);
			if (isColliedTri8)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri9 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v5, OctreeIndex.v4, OctreeIndex.v6, &fDist);
			if (isColliedTri9)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri10 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v0, OctreeIndex.v1, OctreeIndex.v3, &fDist);
			if (isColliedTri10)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			isColliedTri11 = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v3, OctreeIndex.v2, OctreeIndex.v0, &fDist);
			if (isColliedTri11)
			{
				if (SelectTile(pRoot, vOrigin, vDir, fDist, i))
				{
					return;
				}
			}

			//if (!(isColliedTri0 == false
			//	&& isColliedTri1 == false
			//	&& isColliedTri2 == false
			//	&& isColliedTri3 == false
			//	&& isColliedTri4 == false
			//	&& isColliedTri5 == false
			//	&& isColliedTri6 == false
			//	&& isColliedTri7 == false
			//	&& isColliedTri8 == false
			//	&& isColliedTri9 == false
			//	&& isColliedTri10 == false
			//	&& isColliedTri11 == false))
			//{
			//	XMFLOAT3 v3PickingPoint;
			//	XMVECTOR vPickingPoint;
			//	vPickingPoint = vOrigin + (fDist * vDir);
			//	XMStoreFloat3(&v3PickingPoint, vPickingPoint);

			//	float fCubeHeight = v3PickingPoint.y;
			//	float fMapHeight = m_vecVertex[(v3PickingPoint.x) + (256 - v3PickingPoint.z)*(MAP_SIZE)].Pos.y;

			//	if (fMapHeight >= fCubeHeight)
			//	{
			//		m_vPickingPoint = v3PickingPoint;
			//		pRoot = pRoot->GetChild()[i];
			//		CalPoint(pRoot, vOrigin, vDir, fDist);
			//		return;
			//	}
			//}
		}
	}

	//m_vPickingPoint = pRoot->GetCenterVector();
}

bool cMousePicking::SelectTile(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist, int n)
{
	XMFLOAT3 v3PickingPoint;
	XMVECTOR vPickingPoint;
	vPickingPoint = vOrigin + (fDist * vDir);
	XMStoreFloat3(&v3PickingPoint, vPickingPoint);

	float fCubeHeight = v3PickingPoint.y;
	float fMapHeight = m_vecVertex[(v3PickingPoint.x) + (256 - v3PickingPoint.z)*(MAP_SIZE)].Pos.y;
	 
	if (fMapHeight >= fCubeHeight)
	{
		if (pRoot->GetCorner()[1] - pRoot->GetCorner()[0] == 64)
		{
			XMVECTOR vPoint = CulDataPicking(pRoot->GetCorner()[0],
				pRoot->GetCorner()[2], 64, vOrigin, vDir);
			XMStoreFloat3(&m_vPickingPoint, vPoint);
			return true;
		}
		else
		{
			pRoot = pRoot->GetChild()[n];
			CalPoint(pRoot, vOrigin, vDir, fDist);
			return true;
		}

	}
	else
	{
		return false;
	}
}

XMVECTOR cMousePicking::CulDataPicking(int nIndexFirst, int nIndexSecond, int nRange, XMVECTOR vOrigin, XMVECTOR vDir)
{
	std::vector<XMVECTOR> vecPoint;

	for (int j = 0; j < nRange - 1; j++)
	{
		for (int i = 0; i < nRange - 1; i++)
		{
			bool isColliedA = false;
			bool isColliedB = false;
			float fDistA = 0.f;
			float fDistB = 0.f;

			XMVECTOR v0 = XMLoadFloat3(&m_vecVertex[nIndexFirst + i + 0 + 257 * j].Pos);
			XMVECTOR v1 = XMLoadFloat3(&m_vecVertex[nIndexFirst + i + 1 + 257 * j].Pos);
			XMVECTOR v2 = XMLoadFloat3(&m_vecVertex[nIndexFirst + i + 0 + 257 * (j + 1)].Pos);
			XMVECTOR v3 = XMLoadFloat3(&m_vecVertex[nIndexFirst + i + 1 + 257 * (j + 1)].Pos);

			XMFLOAT3 v30;
			XMFLOAT3 v31;
			XMFLOAT3 v32;
			XMFLOAT3 v33;

			XMStoreFloat3(&v30, v0);
			XMStoreFloat3(&v31, v1);
			XMStoreFloat3(&v32, v2);
			XMStoreFloat3(&v33, v3);

			v30.z = 256 - v30.z;
			v31.z = 256 - v31.z;
			v32.z = 256 - v32.z;
			v33.z = 256 - v33.z;

			v30.y = m_vecVertex[v30.x + (256 - v30.z)*(MAP_SIZE)].Pos.y;
			v31.y = m_vecVertex[v31.x + (256 - v31.z)*(MAP_SIZE)].Pos.y;
			v32.y = m_vecVertex[v32.x + (256 - v32.z)*(MAP_SIZE)].Pos.y;
			v33.y = m_vecVertex[v33.x + (256 - v33.z)*(MAP_SIZE)].Pos.y;

			v0 = XMLoadFloat3(&v30);
			v1 = XMLoadFloat3(&v31);
			v2 = XMLoadFloat3(&v32);
			v3 = XMLoadFloat3(&v33);

			isColliedA = XNA::IntersectRayTriangle(vOrigin, vDir, v0, v2, v3, &fDistA);
			isColliedB = XNA::IntersectRayTriangle(vOrigin, vDir, v3, v1, v0, &fDistB);

			if (isColliedA)
			{
				XMFLOAT3 v3PickingPoint;
				XMVECTOR vPickingPoint;
				vPickingPoint = vOrigin + (fDistA * vDir);

				vecPoint.push_back(vPickingPoint);
			}

			if (isColliedB)
			{
				XMFLOAT3 v3PickingPoint;
				XMVECTOR vPickingPoint;
				vPickingPoint = vOrigin + (fDistB * vDir);

				vecPoint.push_back(vPickingPoint);
			}

		}
	}

	if (vecPoint.size() >= 1)
	{
		return GetNearPoint(vecPoint);
	}
}

bool cMousePicking::HeightEdit()
{
    if (m_eKeyReturn == E_KEYDOWN)
    {
        if (m_StackPrve.size() > 0)
        {
            m_vecVertex = m_StackPrve.top();
            m_StackPrve.pop();
        }
        return true;
    }

    if (GetAsyncKeyState('N') & 0x8000)
	{
		m_eEditType = E_INCREASE;
	}
    else if(GetAsyncKeyState('M') & 0x8000)
	{
		m_eEditType = E_DECREASE;
	}
    else if(GetAsyncKeyState('V') & 0x8000)
    {
        m_eEditType = E_NORMALIZE;
    }
    else if (GetAsyncKeyState('C') & 0x8000)
    {
        m_eEditType = E_TEST;
    }
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		m_eEditType = E_EMPTY;
	}
	if (m_eEditType != E_EMPTY)
	{
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
            if (m_eKeyTest == E_KEYDOWN)
            {
                m_StackPrve.push(m_vecVertex);
				if (m_StackPrve.size() >= 10)
				{
					while (m_StackPrve.size())
					{
						m_StackPrve.pop();
					}
				}
            }
            else
            {
                CalGauss(m_vPickingPoint.x, m_vPickingPoint.z, m_fRho);
            }

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool cMousePicking::TextureMap()
{
	if (GetAsyncKeyState('1') & 0x8000)
	{
		m_eTextureType = DirectX::E_GRASS;
	}
	else if (GetAsyncKeyState('2') & 0x8000)
	{
		m_eTextureType = DirectX::E_DARKDIRT;
	}
	else if (GetAsyncKeyState('3') & 0x8000)
	{
		m_eTextureType = DirectX::E_STONE;
	}
	else if (GetAsyncKeyState('4') & 0x8000)
	{
		m_eTextureType = DirectX::E_LIGHTDIRT;
	}
	else if (GetAsyncKeyState('5') & 0x8000)
	{
		m_eTextureType = DirectX::E_SNOW;
	}
	if (m_eTextureType != DirectX::E_ALPHAEMPTY)
	{
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			if (m_isRightClick)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	}
	else
	{
		return false;
	}

}

std::vector<Vertex::ST_P_VERTEX> cMousePicking::GetHeight()
{
	return m_vecVertex;
}

float cMousePicking::GetGaussian(float fX, float fZ, float fRho)
{
	float g = 1.0f / sqrt(2.0f * D3DX_PI * fRho * fRho);
	return g * exp(-(fX * fX + fZ * fZ) / (2 * fRho * fRho));
}

std::vector<D3DXVECTOR3> cMousePicking::GetVecPoint()
{
	return m_vecPoint;
}

DirectX::TextureType cMousePicking::GetTextureType()
{
	return m_eTextureType;
}

void cMousePicking::CalGauss(int nX, int nZ, float fDelta)
{
	float fSizeCheck = 1.f;
	int nRange = 0.f;
	while (fSizeCheck > 0.0001f)
	{
		nRange++;
		fSizeCheck = GetGaussian(nRange, 0.f, 1.f + sqrt(sqrt(fDelta)));
	}

	int nGaussX = -nRange;
	int nGaussZ = -nRange;

    float fAverage = 0.f;
    int nSize = 0;
    for (int z = nZ - nRange; z <= nZ + nRange; z++)
    {
        for (int x = nX - nRange; x <= nX + nRange; x++)
        {
			if (x + (MAP_SIZE - z)*MAP_SIZE < 0 || x + (MAP_SIZE - z)*MAP_SIZE >= m_vecVertex.size())
			{
				return;
			}
            fAverage += m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y;
            nSize++;
        }
    }
    fAverage /= nSize;


	for (int z = nZ - nRange; z <= nZ + nRange; z++)
	{
		nGaussZ++;
		for (int x = nX - nRange; x <= nX + nRange; x++)
		{
			nGaussX++;
			fSizeCheck = GetGaussian(nGaussX, nGaussZ, 1.f + sqrt(sqrt(fDelta)));
			if (x + (MAP_SIZE - z)*MAP_SIZE < MAP_SIZE*MAP_SIZE)
			{
				if (m_eEditType == E_INCREASE)
				{
					m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y += fSizeCheck * sqrt(fDelta);
				}
				if (m_eEditType == E_DECREASE)
				{
					if (m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y - fSizeCheck * sqrt(fDelta) >= fSizeCheck * sqrt(fDelta))
					{
						m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y -= fSizeCheck * sqrt(fDelta);
					}
				}
                if (m_eEditType == E_NORMALIZE)
                {
                    if (m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y + fSizeCheck * sqrt(fDelta) > fAverage)
                    {
                        m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y -=
                            fSizeCheck * sqrt(fDelta)*0.03f;
                    }
                    else if (m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y - fSizeCheck * sqrt(fDelta) < fAverage)
                    {
                        m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y += 
                            fSizeCheck * sqrt(fDelta)*0.03f;
                    }
                    else
                    {
                        return;
                    }
                }
			}
		}
		nGaussX = -nRange;
	}
}

void cMousePicking::SelectCircle(int nX, int nZ, int nRange)
{
    int nSize = (nRange * 2+1)*(nRange * 2+1);
    std::vector<D3DXVECTOR3> vecVertex;
    vecVertex.reserve(nSize);

    for (int z = nZ - nRange+1; z < nZ + nRange; ++z)
    {
        for (int x = nX - nRange+1; x < nX + nRange; ++x)
        {
            D3DXVECTOR3 vec3(0.f, 0.f, 0.f);
            vec3.x = x;
            vec3.z = z;
            vecVertex.push_back(vec3);
        }
    }

	std::vector<float> vecGauss;
	for (int i = 0; i < vecVertex.size(); i++)
	{
		float fGauss = GetGaussian(vecVertex[i].x-nX, vecVertex[i].z-nZ, 2.0f);
		vecGauss.push_back(fGauss);
	}


	m_vecPoint = vecVertex;
	m_vecDepth = vecGauss;
    //std::vector<D3DXVECTOR3> vecReturn;

    //for (int i = 0; i < vecVertex.size(); i++)
    //{
    //    D3DXVECTOR3 vFrom(0.f, 0.f, 0.f);
    //    D3DXVECTOR3 vTo(0.f, 0.f, 0.f);
    //    D3DXVECTOR3 vDist(0.f, 0.f, 0.f);

    //    vFrom.x = nX;
    //    vFrom.z = nZ;

    //    vTo.x = vecVertex[i].x;
    //    vTo.z = vecVertex[i].z;

    //    vDist = vFrom - vTo;

    //    float fDist = D3DXVec3Length(&vDist);

    //    if (fDist <= nRange)
    //    {
    //        D3DXVECTOR3 v(0.f, 0.f, 0.f);
    //        v = vecVertex[i];

    //        vecReturn.push_back(v);
    //    }
    //}

}

void cMousePicking::SetMappingData()
{
	DirectX::ST_PD_VERTEX stData;
	stData.vecPoint = m_vecPoint;
	stData.vecDepth = m_vecDepth;
	stData.eType = m_eTextureType;

	m_pdVertex = stData;
}

DirectX::ST_PD_VERTEX cMousePicking::GetMappingData()
{
	return m_pdVertex;
}

XMVECTOR cMousePicking::GetNearPoint(std::vector<XMVECTOR> vecPoint)
{
	XMVECTOR returnVector;
	float returnDist = 0.0f;
	XMVECTOR vCamera = XMLoadFloat3(&g_pCamera->GetPosition());	

	for (int i = 0; i < vecPoint.size(); i++)
	{
		XMVECTOR vDist = vCamera - vecPoint[i];
		XMFLOAT3 v3Dist;
		XMStoreFloat3(&v3Dist, vDist);
		D3DXVECTOR3 d3Dist(v3Dist.x, v3Dist.y, v3Dist.z);
		float fDist = D3DXVec3Length(&d3Dist);

		if (returnDist == 0.0f)
		{
			returnDist = fDist;
			returnVector = vecPoint[i];
		}
		else if (returnDist >= fDist)
		{
			returnDist = fDist;
			returnVector = vecPoint[i];
		}
	}
	return returnVector;
}