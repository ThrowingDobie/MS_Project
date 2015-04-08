#include "stdafx.h"
#include "cMain.h"
#include "cCube.h"
#include "cLandMark.h"
#include "cSkull.h"

cMain::cMain()
	: m_pCube(NULL)
	, m_pLandMark(NULL)
    , m_pSkull(NULL)
{

}

cMain::~cMain()
{
	g_pD3DDevice->m_pDevCon->ClearState();
	SAFE_DELETE(m_pCube);
	SAFE_DELETE(m_pLandMark);
    SAFE_DELETE(m_pSkull);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

void cMain::Setup()
{
	m_pCube = new cCube;
	m_pCube->Setup();

	m_pLandMark = new cLandMark;
	m_pLandMark->Setup();

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
}

void cMain::Update(float fDelta)
{
	if (m_pCube)
	{
		m_pCube->Update(fDelta);
	}
	if (m_pLandMark)
	{
		m_pLandMark->Update(fDelta);
	}

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

	if (m_pCube)
	{
		m_pCube->Render();
	}
	if (m_pLandMark)
	{
		m_pLandMark->Render();
		m_pLandMark->DrawTreeSprites();
		//m_pLandMark->TessellationRender();
	}

	//if (m_pSkull)
	//{
	//	m_pSkull->Render();
	//}

	g_pD3DDevice->m_pSwapChain->Present(0, 0);
}
