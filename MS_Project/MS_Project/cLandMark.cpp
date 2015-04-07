#include "stdafx.h"
#include "cLandMark.h"


cLandMark::cLandMark()
	: m_pLandVertexBuffer(NULL)
	, m_pLandIndexBuffer(NULL)
	, m_pGrassMapSRV(NULL)
	, m_pWavesVertexBuffer(NULL)
	, m_pWavesIndexBuffer(NULL)
	, m_pWavesMapSRV(NULL)
	, m_pTreeSpritesVB(NULL)
	, m_pTreeTextureMapArraySRV(NULL)
	, m_pQuadPatchVertexBuffer(NULL)
{
	m_nLandIndexCount = 0;
	m_RenderOptions = Lighting;

	m_isAlphaToCoverageOn = false;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_matWaterTexTransform, I);
	XMStoreFloat4x4(&m_matGrassTexTransform, I);
	XMStoreFloat4x4(&m_matLandWorld, I);
	XMStoreFloat4x4(&m_matWavesWorld, I);

}


cLandMark::~cLandMark()
{
	SAFE_RELESE(m_pLandVertexBuffer);
	SAFE_RELESE(m_pLandIndexBuffer);
	SAFE_RELESE(m_pGrassMapSRV);
	SAFE_RELESE(m_pWavesVertexBuffer);
	SAFE_RELESE(m_pWavesIndexBuffer);
	SAFE_RELESE(m_pWavesMapSRV);
	SAFE_RELESE(m_pTreeSpritesVB);
	SAFE_RELESE(m_pTreeTextureMapArraySRV);
	SAFE_RELESE(m_pQuadPatchVertexBuffer);
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
	m_DirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_DirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&m_matGrassTexTransform, grassTexScale);

	m_mtLand.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtLand.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtLand.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_mtWaves.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtWaves.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	m_mtWaves.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	m_Waves.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
		L"Textures/grass.dds", 0, 0, &m_pGrassMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
		L"Textures/water2.dds", 0, 0, &m_pWavesMapSRV, 0));

	m_mtTree.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtTree.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtTree.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	std::vector<std::wstring> treeFilenames;
	treeFilenames.push_back(L"Textures/tree0.dds");
	treeFilenames.push_back(L"Textures/tree1.dds");
	treeFilenames.push_back(L"Textures/tree2.dds");
	treeFilenames.push_back(L"Textures/tree3.dds");

	m_pTreeTextureMapArraySRV = d3dHelper::CreateTexture2DArraySRV(
		g_pD3DDevice->m_pDevice, g_pD3DDevice->m_pDevCon, treeFilenames, DXGI_FORMAT_R8G8B8A8_UNORM);



}

void cLandMark::Init()
{
	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildTreeSpritesBuffer();
	BuildQuadPatchBuffer();
}

void cLandMark::BuildTreeSpritesBuffer()
{
	Vertex::TreePointSprite v[m_nTreeCount];

	for (UINT i = 0; i < m_nTreeCount; ++i)
	{
		float x = MathHelper::RandF(-35.0f, 35.0f);
		float z = MathHelper::RandF(-35.0f, 35.0f);
		float y = GetHillHeight(x, z);

		// Move tree slightly above land height.
		y += 10.0f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(24.0f, 24.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * m_nTreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pTreeSpritesVB));
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

	if (GetAsyncKeyState('R') & 0x8000)
		m_isAlphaToCoverageOn = true;

	if (GetAsyncKeyState('T') & 0x8000)
		m_isAlphaToCoverageOn = false;
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
	Effects::BasicFX->SetDirLights(m_DirLights);
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

void cLandMark::DrawTreeSprites()
{
	XMMATRIX view = XMLoadFloat4x4(&g_pCamera->m_matView);
	XMMATRIX proj = XMLoadFloat4x4(&g_pCamera->m_matProj);
	XMMATRIX viewProj = view*proj;

	Effects::TreeSpriteFX->SetDirLights(m_DirLights);
	Effects::TreeSpriteFX->SetEyePosW(g_pCamera->m_vEyePosW);
	Effects::TreeSpriteFX->SetFogColor(Colors::Silver);
	Effects::TreeSpriteFX->SetFogStart(15.0f);
	Effects::TreeSpriteFX->SetFogRange(175.0f);
	Effects::TreeSpriteFX->SetViewProj(viewProj);
	Effects::TreeSpriteFX->SetMaterial(m_mtTree);
	Effects::TreeSpriteFX->SetTreeTextureMapArray(m_pTreeTextureMapArraySRV);

	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::TreePointSprite);
	UINT stride = sizeof(Vertex::TreePointSprite);
	UINT offset = 0;

	ID3DX11EffectTechnique* treeTech = nullptr;
	switch (m_RenderOptions)
	{
	case RenderOptions::Lighting:
		treeTech = Effects::TreeSpriteFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;
		break;
	case RenderOptions::TexturesAndFog:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pTreeSpritesVB, &stride, &offset);

		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (m_isAlphaToCoverageOn)
		{
			g_pD3DDevice->m_pDevCon->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
		}
		treeTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);
		g_pD3DDevice->m_pDevCon->Draw(m_nTreeCount, 0);

		g_pD3DDevice->m_pDevCon->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}

void cLandMark::TessellationRender()
{
	g_pD3DDevice->m_pDevCon->ClearRenderTargetView(g_pD3DDevice->m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	g_pD3DDevice->m_pDevCon->ClearDepthStencilView(g_pD3DDevice->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::BasicTess);
	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX view = XMLoadFloat4x4(&g_pCamera->m_matView);
	XMMATRIX proj = XMLoadFloat4x4(&g_pCamera->m_matProj);
	XMMATRIX viewProj = view*proj;

	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::Pos);
	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(Vertex::Pos);
	UINT offset = 0;

	// Set per frame constants.
	Effects::TessellationFX->SetEyePosW(g_pCamera->m_vEyePosW);
	Effects::TessellationFX->SetFogColor(Colors::Silver);
	Effects::TessellationFX->SetFogStart(15.0f);
	Effects::TessellationFX->SetFogRange(175.0f);

	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::TessellationFX->TessTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pQuadPatchVertexBuffer, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMMatrixIdentity();
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::TessellationFX->SetWorld(world);
		Effects::TessellationFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::TessellationFX->SetWorldViewProj(worldViewProj);
		Effects::TessellationFX->SetTexTransform(XMMatrixIdentity());
		//Effects::TessellationFX->SetMaterial(0);
		Effects::TessellationFX->SetDiffuseMap(0);

		Effects::TessellationFX->TessTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);

		g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::WireframeRS);
		g_pD3DDevice->m_pDevCon->Draw(4, 0);
	}
}

void cLandMark::BuildQuadPatchBuffer()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	XMFLOAT3 vertices[4] =
	{
		XMFLOAT3(-10.0f, 0.0f, +10.0f),
		XMFLOAT3(+10.0f, 0.0f, +10.0f),
		XMFLOAT3(-10.0f, 0.0f, -10.0f),
		XMFLOAT3(+10.0f, 0.0f, -10.0f)
	};

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pQuadPatchVertexBuffer));
}