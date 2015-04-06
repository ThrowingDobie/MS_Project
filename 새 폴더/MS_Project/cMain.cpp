#include "stdafx.h"
#include "cMain.h"
#include "cCube.h"
#include "cLandMark.h"

cMain::cMain()
	: m_pCube(NULL)
	, m_pLandMark(NULL)
{
}


cMain::~cMain()
{
	SAFE_DELETE(m_pCube);
	SAFE_DELETE(m_pLandMark);
}

void cMain::Setup()
{
	LightSetup();

	m_pCube = new cCube;
	m_pCube->Setup();

	m_pLandMark = new cLandMark;
	m_pLandMark->Setup();
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
}

void cMain::Render()
{
	XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	g_pD3DDevice->m_pDevCon->ClearRenderTargetView(g_pD3DDevice->m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	g_pD3DDevice->m_pDevCon->ClearDepthStencilView(g_pD3DDevice->m_pDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	Effects::BasicFX->SetDirLights(m_DirLights);

	if (m_pCube)
	{
		m_pCube->Render();
	}
	if (m_pLandMark)
	{
		m_pLandMark->Render();
	}

	g_pD3DDevice->m_pSwapChain->Present(0, 0);
}

void cMain::LightSetup()
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

}