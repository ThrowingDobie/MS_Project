#include "stdafx.h"
#include "cMain.h"
#include "cCube.h"
#include "cLandMark.h"
#include "cSkull.h"
#include "cTerrain.h"

cMain::cMain()
	: m_pCube(NULL)
	, m_pLandMark(NULL)
    , m_pSkull(NULL)
	, m_pTerrain(NULL)
{

}

cMain::~cMain()
{
	SAFE_DELETE(m_pCube);
	SAFE_DELETE(m_pLandMark);
    SAFE_DELETE(m_pSkull);
	SAFE_DELETE(m_pTerrain);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

void cMain::Setup()
{
	LightSetup();

	m_pTerrain = new cTerrain;

	g_pMousePicking->Setup();

}

void cMain::Init()
{
    Effects::InitAll(g_pD3DDevice->m_pDevice);
    InputLayouts::InitAll(g_pD3DDevice->m_pDevice);
    RenderStates::InitAll(g_pD3DDevice->m_pDevice);

	cTerrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/test.raw";
	tii.LayerMapFilename0 = L"Textures/snow.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/stone.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/grass.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 30.f;
	tii.HeightmapWidth = 257;
    tii.HeightmapHeight = 257;
	tii.CellSpacing = 0.5f;

	m_pTerrain->Init(g_pD3DDevice->m_pDevice, g_pD3DDevice->m_pDevCon, tii);

	g_pMousePicking->Init(m_pTerrain->m_pQuadPatchVertexBuffer
		, m_pTerrain->m_pQuadPatchIndexBuffer
		, &m_pTerrain->GetWorld()
		, m_pTerrain->m_vecHeightmap);
}

void cMain::Update(float fDelta)
{
    if (GetAsyncKeyState('1') & 0x8000)
    {
        g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::WireframeRS);
    }
    if (GetAsyncKeyState('2') & 0x8000)
    {
        g_pD3DDevice->m_pDevCon->RSSetState(NULL);
    }
}

void cMain::Render()
{
	XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	g_pD3DDevice->m_pDevCon->ClearRenderTargetView(g_pD3DDevice->m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	g_pD3DDevice->m_pDevCon->ClearDepthStencilView(g_pD3DDevice->m_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (m_pTerrain)
	{
		m_pTerrain->Render(g_pD3DDevice->m_pDevCon, *g_pCamera, m_DirLights);
	}

	g_pMousePicking->Render(m_DirLights);

	g_pD3DDevice->m_pSwapChain->Present(0, 0);
}

void cMain::LightSetup()
{
	m_DirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_DirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	m_DirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[1].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);
}
