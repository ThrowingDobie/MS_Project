#include "stdafx.h"
#include "cLandMark.h"


cLandMark::cLandMark()
{
	m_nLandIndexCount = 0;
	m_RenderOptions = Lighting;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_matWaterTexTransform, I);
	XMStoreFloat4x4(&m_matGrassTexTransform, I);
	XMStoreFloat4x4(&m_matLandWorld, I);
	XMStoreFloat4x4(&m_matWavesWorld, I);
}


cLandMark::~cLandMark()
{
}

void cLandMark::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	m_nLandIndexCount = grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
	for (UINT i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pLandVertexBuffer));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_nLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_pLandIndexBuffer));
}

void cLandMark::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * m_Waves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, 0, &m_pWavesVertexBuffer));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3 * m_Waves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = m_Waves.RowCount();
	UINT n = m_Waves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_pWavesIndexBuffer));
}

float cLandMark::GetHillHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 cLandMark::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void cLandMark::Setup()
{
	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&m_matGrassTexTransform, grassTexScale);

	m_mtLand.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtLand.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtLand.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_mtWaves.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtWaves.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	m_mtWaves.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	m_Waves.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(g_pD3DDevice->m_pDevice);
	InputLayouts::InitAll(g_pD3DDevice->m_pDevice);
	RenderStates::InitAll(g_pD3DDevice->m_pDevice);

	HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
		L"Textures/grass.dds", 0, 0, &m_pGrassMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
		L"Textures/water2.dds", 0, 0, &m_pWavesMapSRV, 0));

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
}

void cLandMark::Update(float fDelta)
{
	static float t_base = 0.0f;
	if ((g_pTimer->TotalTime() - t_base) >= 0.1f)
	{
		t_base += 0.1f;

		DWORD i = 5 + rand() % (m_Waves.RowCount() - 10);
		DWORD j = 5 + rand() % (m_Waves.ColumnCount() - 10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		m_Waves.Disturb(i, j, r);
	}

	m_Waves.Update(fDelta);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(g_pD3DDevice->m_pDevCon->Map(m_pWavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for (UINT i = 0; i < m_Waves.VertexCount(); ++i)
	{
		v[i].Pos = m_Waves[i];
		v[i].Normal = m_Waves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x = 0.5f + m_Waves[i].x / m_Waves.Width();
		v[i].Tex.y = 0.5f - m_Waves[i].z / m_Waves.Depth();
	}

	g_pD3DDevice->m_pDevCon->Unmap(m_pWavesVertexBuffer, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	m_WaterTexOffset.y += 0.05f*fDelta;
	m_WaterTexOffset.x += 0.1f*fDelta;
	XMMATRIX wavesOffset = XMMatrixTranslation(m_WaterTexOffset.x, m_WaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&m_matWaterTexTransform, wavesScale*wavesOffset);

	//
	// Switch the render mode based in key input.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		m_RenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		m_RenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		m_RenderOptions = RenderOptions::TexturesAndFog;
}

void cLandMark::Render()
{
	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::Basic32);
	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&g_pCamera->m_matView);
	XMMATRIX proj = XMLoadFloat4x4(&g_pCamera->m_matProj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetEyePosW(g_pCamera->m_vEyePosW);
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(175.0f);

	ID3DX11EffectTechnique* landAndWavesTech;
	landAndWavesTech = nullptr;

	switch (m_RenderOptions)
	{
	case RenderOptions::Lighting:
		landAndWavesTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		landAndWavesTech = Effects::BasicFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	landAndWavesTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//
		// Draw the hills.
		//
		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pLandVertexBuffer, &stride, &offset);
		g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pLandIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&m_matLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&m_matGrassTexTransform));
		Effects::BasicFX->SetMaterial(m_mtLand);
		Effects::BasicFX->SetDiffuseMap(m_pGrassMapSRV);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);
		g_pD3DDevice->m_pDevCon->DrawIndexed(m_nLandIndexCount, 0, 0);

		//
		// Draw the waves.
		//
		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pWavesVertexBuffer, &stride, &offset);
		g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pWavesIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.

		world = XMLoadFloat4x4(&m_matWavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&m_matWaterTexTransform));
		Effects::BasicFX->SetMaterial(m_mtWaves);
		Effects::BasicFX->SetDiffuseMap(m_pWavesMapSRV);

		g_pD3DDevice->m_pDevCon->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		landAndWavesTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);
		g_pD3DDevice->m_pDevCon->DrawIndexed(3 * m_Waves.TriangleCount(), 0, 0);

		// Restore default blend state
		g_pD3DDevice->m_pDevCon->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}