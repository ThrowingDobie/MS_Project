#include "stdafx.h"
#include "cMousePicking.h"

#define MAP_SIZE 257


cMousePicking::cMousePicking()
	: m_pVertexBuffer(NULL)
	, m_pIndexBuffer(NULL)
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
}

cMousePicking::~cMousePicking()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
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

void cMousePicking::Update(float fDelta)
{
	if (HeightEdit())
	{
		m_vecSave = m_vecVertex;
	}
	
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
		Pick(nX, nY);
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

	XMMATRIX W = XMLoadFloat4x4(&m_matMeshWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	rayDir = XMVector3Normalize(rayDir);
	//

	for (int i = 0; i < m_vecVertex.size() - (MAP_SIZE + 2); i++)
	{
		if (i % MAP_SIZE != MAP_SIZE - 1)
		{
			UINT i0 = (i + 0); 
			UINT i1 = (i + 1);
			UINT i2 = (i + 1) + MAP_SIZE;

			XMVECTOR v0 = XMLoadFloat3(&m_vecVertex[i0].Pos);
			XMVECTOR v1 = XMLoadFloat3(&m_vecVertex[i1].Pos);
			XMVECTOR v2 = XMLoadFloat3(&m_vecVertex[i2].Pos);

			float fDist = 0.f;
			bool isColliedTri = false;
			isColliedTri = XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &fDist);

			if (isColliedTri == true)
			{
				XMVECTOR vPickingPoint;
				vPickingPoint = rayOrigin + (fDist*rayDir);
				XMStoreFloat3(&m_vPickingPoint, vPickingPoint);
				//return;
			}

			i0 = (i + 1) + MAP_SIZE;
			i1 = (i + 0) + MAP_SIZE;
			i2 = (i + 0);

			v0 = XMLoadFloat3(&m_vecVertex[i0].Pos);
			v1 = XMLoadFloat3(&m_vecVertex[i1].Pos);
			v2 = XMLoadFloat3(&m_vecVertex[i2].Pos);

			fDist = 0.f;
			isColliedTri = XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &fDist);

			if (isColliedTri == true)
			{
				XMVECTOR vPickingPoint;
				vPickingPoint = rayOrigin + (fDist*rayDir);
				XMStoreFloat3(&m_vPickingPoint, vPickingPoint);
				//return;
			}
		}
	}
}

bool cMousePicking::HeightEdit()
{
	if (GetAsyncKeyState('P') & 0x8000)
	{
		m_vecVertex = m_vecSave;
		return true;
	}
	if (GetAsyncKeyState('N') & 0x8000)
	{
		m_eEditType = E_INCREASE;
	}
	if (GetAsyncKeyState('M') & 0x8000)
	{
		m_eEditType = E_DECREASE;
	}
	if (GetAsyncKeyState('B') & 0x8000)
	{
		m_eEditType = E_ERASE;
	}
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		m_eEditType = E_EMPTY;
	}
	if (m_eEditType != E_EMPTY)
	{
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			CalGauss(m_vPickingPoint.x, m_vPickingPoint.z, m_fRho);
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

std::vector<Vertex::ST_P_VERTEX> cMousePicking::GetHeight()
{
	return m_vecVertex;
}

float cMousePicking::GetGaussian(float fX, float fZ, float fRho)
{
	float g = 1.0f / sqrt(2.0f * D3DX_PI * fRho * fRho);
	return g * exp(-(fX * fX + fZ * fZ) / (2 * fRho * fRho));
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
				if (m_eEditType == E_ERASE)
				{
					m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y = 0;
				}
			}
		}
		nGaussX = -nRange;
	}
}
//if (m_vecVertex[x + z*(MAP_SIZE)].Pos.y + fSizeCheck * sqrt(fDelta) >= 25.5f)
//{
//	m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y = 25.5f;
//}
//else
//{
//	m_vecVertex[x + (MAP_SIZE - z)*MAP_SIZE].Pos.y += fSizeCheck * sqrt(fDelta);
//}