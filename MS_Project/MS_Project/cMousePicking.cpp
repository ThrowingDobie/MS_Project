#include "stdafx.h"
#include "cMousePicking.h"
#include "cTerrain.h"
#include "cQuadTree.h"
#include "cOctree.h"

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
	m_eTextureUsingType = DirectX::E_USINGNULL;

	m_eTextureType_Mouse = DirectX::E_ALPHAEMPTY;
	m_eTextureUsingType_Mouse = DirectX::E_USINGNULL;

	m_isRightClick = false;

	m_nMapSize = 257;
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
			vertex.Pos.x = (int)(i % m_nMapSize);
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
	//std::wostringstream outs;
	//outs.precision(3);
	//outs << L"X = " << m_vPickingPoint.x << L"   Z = " << m_vPickingPoint.z
	//	<< L"   Y = " << m_vPickingPoint.y;

	//SetWindowText(g_hWnd, outs.str().c_str());

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

		//if (GetAsyncKeyState('1') & 0x8000)
		//	g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::WireframeRS);

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
		SetMouseMappingData();
    }
	else
	{
		m_isRightClick = false;
	}
}


void cMousePicking::BuildMeshGeometryBuffers()
{
	std::ifstream fin("Models/car.txt");

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
	if (nX != 0 && nY != 0)
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

		m_pOctree = new cOctree(m_nMapSize);
		CalPoint(m_pOctree, rayOrigin, rayDir, 0);
		if (m_vecColliedTri3.size() > 0)
		{
			XMStoreFloat3(&m_vPickingPoint, GetNearPointTest(m_vecColliedTri3));
		}
		m_vecColliedTri3.clear();
		SAFE_DELETE(m_pOctree);
	}
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
				CalTail(pRoot, vOrigin, vDir, fDist);
				return false;

			}

			isColliedTri = XNA::IntersectRayTriangle(vOrigin, vDir,
				vecIndex[i].v3, vecIndex[i].v1, vecIndex[i].v0, &fDist);

			if (isColliedTri)
			{
				pRoot = pRoot->m_vecChild[i];
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
			std::vector<bool> vecIsColliedTri;
			vecIsColliedTri.resize(12);

			OctreeIndex.i0 = pRoot->GetChild()[i]->GetIndex()[0];
			OctreeIndex.i1 = pRoot->GetChild()[i]->GetIndex()[1];
			OctreeIndex.i2 = pRoot->GetChild()[i]->GetIndex()[2];
			OctreeIndex.i3 = pRoot->GetChild()[i]->GetIndex()[3];
			OctreeIndex.i4 = pRoot->GetChild()[i]->GetIndex()[4];
			OctreeIndex.i5 = pRoot->GetChild()[i]->GetIndex()[5];
			OctreeIndex.i6 = pRoot->GetChild()[i]->GetIndex()[6];
			OctreeIndex.i7 = pRoot->GetChild()[i]->GetIndex()[7];

			OctreeIndex.v0 = XMLoadFloat3(&SetIndex(OctreeIndex.i0, m_nMapSize));
			OctreeIndex.v1 = XMLoadFloat3(&SetIndex(OctreeIndex.i1, m_nMapSize));
			OctreeIndex.v2 = XMLoadFloat3(&SetIndex(OctreeIndex.i2, m_nMapSize));
			OctreeIndex.v3 = XMLoadFloat3(&SetIndex(OctreeIndex.i3, m_nMapSize));
			OctreeIndex.v4 = XMLoadFloat3(&SetIndex(OctreeIndex.i4, m_nMapSize));
			OctreeIndex.v5 = XMLoadFloat3(&SetIndex(OctreeIndex.i5, m_nMapSize));
			OctreeIndex.v6 = XMLoadFloat3(&SetIndex(OctreeIndex.i6, m_nMapSize));
			OctreeIndex.v7 = XMLoadFloat3(&SetIndex(OctreeIndex.i7, m_nMapSize));

			vecIsColliedTri[0] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v4, OctreeIndex.v5, OctreeIndex.v1, &fDist);
			vecIsColliedTri[1] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v1, OctreeIndex.v0, OctreeIndex.v4, &fDist);
			vecIsColliedTri[2] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v5, OctreeIndex.v7, OctreeIndex.v3, &fDist);
			vecIsColliedTri[3] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v3, OctreeIndex.v1, OctreeIndex.v5, &fDist);
			vecIsColliedTri[4] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v7, OctreeIndex.v3, OctreeIndex.v2, &fDist);
			vecIsColliedTri[5] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v2, OctreeIndex.v6, OctreeIndex.v7, &fDist);
			vecIsColliedTri[6] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v6, OctreeIndex.v4, OctreeIndex.v0, &fDist);
			vecIsColliedTri[7] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v0, OctreeIndex.v2, OctreeIndex.v6, &fDist);
			vecIsColliedTri[8] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v6, OctreeIndex.v7, OctreeIndex.v5, &fDist);
			vecIsColliedTri[9] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v5, OctreeIndex.v4, OctreeIndex.v6, &fDist);
			vecIsColliedTri[10] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v0, OctreeIndex.v1, OctreeIndex.v3, &fDist);
			vecIsColliedTri[11] = XNA::IntersectRayTriangle(vOrigin, vDir,
				OctreeIndex.v3, OctreeIndex.v2, OctreeIndex.v0, &fDist);

			for (int i = 0; i < vecIsColliedTri.size(); i++)
			{
				if (vecIsColliedTri[i] == true)
				{
					bool isNext = SelectTile(pRoot, vOrigin, vDir, fDist);
					if (isNext == true && pRoot->GetIsVisible() == true)
					{
						pRoot = pRoot->GetChild()[i];
						CalPoint(pRoot, vOrigin, vDir, fDist);
						return;
					}
				}
			}
		}
	}
}

bool cMousePicking::SelectTile(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist)
{
	int nSize = pRoot->GetCorner()[1] - pRoot->GetCorner()[0];
	bool isCollied = CulDataPicking(pRoot->GetCorner()[0], nSize, vOrigin, vDir);

	if (isCollied == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool cMousePicking::CulDataPicking(int nIndexFirst, int nRange, XMVECTOR vOrigin, XMVECTOR vDir)
{
	int nVectorSize = 0;

	for (int j = 0; j < nRange; j++)
	{
		for (int i = 0; i < nRange; i++)
		{
			bool isColliedA = false;
			bool isColliedB = false;
			float fDistA = 0.f;
			float fDistB = 0.f;

			if (nIndexFirst + (i + 1) + m_nMapSize * (j + 1) > m_vecVertex.size() - 1)
			{
				break;
			}

			XMVECTOR v0 = XMLoadFloat3(&m_vecVertex[nIndexFirst + (i + 0) + m_nMapSize * (j + 0)].Pos);
			XMVECTOR v1 = XMLoadFloat3(&m_vecVertex[nIndexFirst + (i + 1) + m_nMapSize * (j + 0)].Pos);
			XMVECTOR v2 = XMLoadFloat3(&m_vecVertex[nIndexFirst + (i + 0) + m_nMapSize * (j + 1)].Pos);
			XMVECTOR v3 = XMLoadFloat3(&m_vecVertex[nIndexFirst + (i + 1) + m_nMapSize * (j + 1)].Pos);

			XMFLOAT3 v30;
			XMFLOAT3 v31;
			XMFLOAT3 v32;
			XMFLOAT3 v33;

			XMStoreFloat3(&v30, v0);
			XMStoreFloat3(&v31, v1);
			XMStoreFloat3(&v32, v2);
			XMStoreFloat3(&v33, v3);

			v30.z = (m_nMapSize - 1) - v30.z;
			v31.z = (m_nMapSize - 1) - v31.z;
			v32.z = (m_nMapSize - 1) - v32.z;
			v33.z = (m_nMapSize - 1) - v33.z;

			v30.y = m_vecVertex[v30.x + ((m_nMapSize - 1) - v30.z)*(m_nMapSize)].Pos.y;
			v31.y = m_vecVertex[v31.x + ((m_nMapSize - 1) - v31.z)*(m_nMapSize)].Pos.y;
			v32.y = m_vecVertex[v32.x + ((m_nMapSize - 1) - v32.z)*(m_nMapSize)].Pos.y;
			v33.y = m_vecVertex[v33.x + ((m_nMapSize - 1) - v33.z)*(m_nMapSize)].Pos.y;

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
				XMStoreFloat3(&v3PickingPoint, vPickingPoint);

				nVectorSize++;
				//m_vecColliedTri.push_back(vPickingPoint);

				m_vecColliedTri3.push_back(v3PickingPoint);
			}

			if (isColliedB)
			{
				XMFLOAT3 v3PickingPoint;
				XMVECTOR vPickingPoint;
				vPickingPoint = vOrigin + (fDistB * vDir);
				XMStoreFloat3(&v3PickingPoint, vPickingPoint);

				nVectorSize++;
				//m_vecColliedTri.push_back(vPickingPoint);

				m_vecColliedTri3.push_back(v3PickingPoint);
			}
		}
	}

	if (nVectorSize > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
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

XMVECTOR cMousePicking::GetNearPointTest(std::vector<XMFLOAT3> vecPoint)
{
	XMVECTOR returnVector;
	float returnDist = 0.0f;
	
	XMVECTOR vCamera = XMLoadFloat3(&g_pCamera->GetPosition());

	for (int i = 0; i < vecPoint.size(); i++)
	{
		XMVECTOR vPoint = XMLoadFloat3(&vecPoint[i]);

		XMVECTOR vDist = vCamera - vPoint;
		XMFLOAT3 v3Dist;
		XMStoreFloat3(&v3Dist, vDist);
		D3DXVECTOR3 d3Dist(v3Dist.x, v3Dist.y, v3Dist.z);
		float fDist = D3DXVec3Length(&d3Dist);

		if (returnDist == 0.0f)
		{
			returnDist = fDist;
			returnVector = vPoint;
		}
		else if (returnDist >= fDist)
		{
			returnDist = fDist;
			returnVector = vPoint;
		}
	}
	return returnVector;
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
		m_eTextureUsingType = DirectX::E_MAPPINGTEXTURE;
		m_eTextureType = DirectX::E_GRASS;
	}
	else if (GetAsyncKeyState('2') & 0x8000)
	{
		m_eTextureUsingType = DirectX::E_MAPPINGTEXTURE;
		m_eTextureType = DirectX::E_DARKDIRT;
	}
	else if (GetAsyncKeyState('3') & 0x8000)
	{
		m_eTextureUsingType = DirectX::E_MAPPINGTEXTURE;
		m_eTextureType = DirectX::E_STONE;
	}
	else if (GetAsyncKeyState('4') & 0x8000)
	{
		m_eTextureUsingType = DirectX::E_MAPPINGTEXTURE;
		m_eTextureType = DirectX::E_LIGHTDIRT;
	}
	else if (GetAsyncKeyState('5') & 0x8000)
	{
		m_eTextureUsingType = DirectX::E_MAPPINGTEXTURE;
		m_eTextureType = DirectX::E_SNOW;
	}
	if (m_eTextureUsingType != DirectX::E_USINGNULL)
	{
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool cMousePicking::MouseRange()
{
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		m_eTextureType_Mouse = DirectX::E_GRASS;
		m_eTextureUsingType_Mouse = DirectX::E_MOUSE;
	}
	if (GetAsyncKeyState('X') & 0x8000)
	{
		return true;
	}
	return false;
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
			if (x + (m_nMapSize - z)*m_nMapSize < 0 || x + (m_nMapSize - z)*m_nMapSize >= 
				m_vecVertex.size())
			{
				return;
			}
			fAverage += m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y;
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
			if (x + (m_nMapSize - z)*m_nMapSize < m_nMapSize*m_nMapSize)
			{
				if (m_eEditType == E_INCREASE)
				{
					m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y += fSizeCheck * sqrt(fDelta);
				}
				if (m_eEditType == E_DECREASE)
				{
					if (m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y - fSizeCheck * sqrt(fDelta) >= fSizeCheck * sqrt(fDelta))
					{
						m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y -= fSizeCheck * sqrt(fDelta);
					}
				}
                if (m_eEditType == E_NORMALIZE)
                {
					if (m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y + fSizeCheck * sqrt(fDelta) > fAverage)
                    {
						m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y -=
                            fSizeCheck * sqrt(fDelta)*0.03f;
                    }
					else if (m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y - fSizeCheck * sqrt(fDelta) < fAverage)
                    {
						m_vecVertex[x + (m_nMapSize - z)*m_nMapSize].Pos.y +=
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
	m_vecPoint_Mouse = vecVertex;
	m_vecDepth_Mouse = vecGauss;
}

void cMousePicking::SetMappingData()
{
	DirectX::ST_PD_VERTEX stData;
	stData.vecPoint = m_vecPoint;
	stData.vecDepth = m_vecDepth;
	stData.eType = m_eTextureType;
	stData.eTextureUsingType = m_eTextureUsingType;

	m_pdVertex = stData;
}

void cMousePicking::SetMouseMappingData()
{
	DirectX::ST_PD_VERTEX stData;
	stData.vecPoint = m_vecPoint_Mouse;
	stData.vecDepth = m_vecDepth_Mouse;
	stData.eType = m_eTextureType_Mouse;
	stData.eTextureUsingType = m_eTextureUsingType_Mouse;

	m_pdVertex_Mouse = stData;
}

DirectX::ST_PD_VERTEX cMousePicking::GetMappingData()
{
	return m_pdVertex;
}

DirectX::ST_PD_VERTEX cMousePicking::GetMouseMappingData()
{
	return m_pdVertex_Mouse;
}