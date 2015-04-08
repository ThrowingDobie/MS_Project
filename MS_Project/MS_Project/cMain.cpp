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
	g_pD3DDevice->m_pDevCon->ClearState();
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

	m_pCube = new cCube;
	m_pCube->Setup();

	m_pLandMark = new cLandMark;
	m_pLandMark->Setup();

	m_pTerrain = new cTerrain;

    //m_pSkull = new cSkull;
    //m_pSkull->Setup();
}

void cMain::Init()
{
    Effects::InitAll(g_pD3DDevice->m_pDevice);
    InputLayouts::InitAll(g_pD3DDevice->m_pDevice);
    RenderStates::InitAll(g_pD3DDevice->m_pDevice);

	m_pLandMark->Init();
    //m_pSkull->Init();

	cTerrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain.raw";
	tii.LayerMapFilename0 = L"Textures/grass.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/stone.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/snow.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;

	m_pTerrain->Init(g_pD3DDevice->m_pDevice, g_pD3DDevice->m_pDevCon, tii);
}

void cMain::Update(float fDelta)
{
	//if (m_pCube)
	//{
	//	m_pCube->Update(fDelta);
	//}
	//if (m_pLandMark)
	//{
	//	m_pLandMark->Update(fDelta);
	//}

    //if (m_pSkull)
    //{
    //    m_pSkull->Update(fDelta);
    //}
}

void cMain::Render()
{
	XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	g_pD3DDevice->m_pDevCon->ClearRenderTargetView(g_pD3DDevice->m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	g_pD3DDevice->m_pDevCon->ClearDepthStencilView(g_pD3DDevice->m_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//if (m_pCube)
	//{
	//	m_pCube->Render();
	//}
	//if (m_pLandMark)
	//{
	//	m_pLandMark->Render();
	//	m_pLandMark->DrawTreeSprites();
	//	//m_pLandMark->TessellationRender();
	//}

	if (GetAsyncKeyState('1') & 0x8000)
		g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::WireframeRS);
	if (m_pTerrain)
	{
		m_pTerrain->Render(g_pD3DDevice->m_pDevCon, *g_pCamera->GetInstance(), m_DirLights);
	}

	//if (m_pSkull)
	//{
	//	m_pSkull->Render();
	//}

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
