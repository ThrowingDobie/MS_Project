#include "stdafx.h"
#include "cCube.h"

cCube::cCube()
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_matBoxWorld, I);

	m_RenderOptions = Lighting;
}


cCube::~cCube()
{
}

void cCube::Setup()
{
	m_mtBox.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mtBox.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtBox.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
		L"Textures/WireFence.dds", 0, 0, &mBoxMapSRV, 0));

	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.0f, -15.0f);
	XMStoreFloat4x4(&m_matBoxWorld, boxScale*boxOffset);

	BuildCrateGeometryBuffers();


}

void cCube::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * box.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(g_pD3DDevice->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer));
}

void cCube::Update(float fDelta)
{
	if (GetAsyncKeyState('1') & 0x8000)
		m_RenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		m_RenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		m_RenderOptions = RenderOptions::TexturesAndFog;
}

void cCube::Render()
{
	g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::Basic32);
	g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&g_pCamera->m_matView);
	XMMATRIX proj = XMLoadFloat4x4(&g_pCamera->m_matProj);
	XMMATRIX viewProj = view*proj;


	ID3DX11EffectTechnique* boxTech;
	boxTech = nullptr;

	switch (m_RenderOptions)
	{
	case RenderOptions::Lighting:
		boxTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
		break;
	case RenderOptions::TexturesAndFog:
		boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	boxTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&m_matBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(m_mtBox);
		Effects::BasicFX->SetDiffuseMap(mBoxMapSRV);

		g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::NoCullRS);
		boxTech->GetPassByIndex(p)->Apply(0, g_pD3DDevice->m_pDevCon);
		g_pD3DDevice->m_pDevCon->DrawIndexed(36, 0, 0);

		// Restore default render state.
		g_pD3DDevice->m_pDevCon->RSSetState(0);
	}
}