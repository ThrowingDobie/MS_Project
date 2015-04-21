#include "stdafx.h"
#include "cDeviceManager.h"

cDeviceManager::cDeviceManager()
{
	m_pDevice = nullptr;
	m_pDevCon = nullptr;
	m_pSwapChain = nullptr;
	m_pStencilBuffer = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDepthStencilView = nullptr;
	m_nClientWidth = 0;
	m_nClientHeight = 0;

	m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	m_nFrameCnt = 0;
	m_fTimeElapsed = 0.f;
}

cDeviceManager::~cDeviceManager()
{
	Destroy();
}

void cDeviceManager::Setup()
{
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	
	m_nClientWidth = rc.right - rc.left;
	m_nClientHeight = rc.bottom - rc.top;
}

void cDeviceManager::Destroy()
{
	if (m_pDevCon)
	{
		m_pDevCon->ClearState();
	}

	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDevCon);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pStencilBuffer);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDepthStencilView);

}

bool cDeviceManager::InitDevice()
{
	// Create the device and device context.

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		m_d3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&m_pDevice,
		&featureLevel,
		&m_pDevCon);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	HR(m_pDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_n4xMsaaQuality));
	assert(m_n4xMsaaQuality > 0);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_nClientWidth;
	sd.BufferDesc.Height = m_nClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (m_isEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_n4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = g_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	HR(m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(m_pDevice, &sd, &m_pSwapChain));

	SAFE_RELEASE(dxgiDevice);
	SAFE_RELEASE(dxgiAdapter);
	SAFE_RELEASE(dxgiFactory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	OnResize();

	return true;
}

void cDeviceManager::OnResize()
{
	HR(m_pSwapChain->ResizeBuffers(1, m_nClientWidth, m_nClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_pDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView));
	SAFE_RELEASE(backBuffer);
	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_nClientWidth;
	depthStencilDesc.Height = m_nClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (m_isEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_n4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pStencilBuffer));
	HR(m_pDevice->CreateDepthStencilView(m_pStencilBuffer, 0, &m_pDepthStencilView));

	// Bind the render target view and depth/stencil view to the pipeline.

	m_pDevCon->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


	// Set the viewport transform.

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_nClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_nClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_pDevCon->RSSetViewports(1, &m_ScreenViewport);
}

void cDeviceManager::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	m_nFrameCnt++;

	// Compute averages over one second period.
	if ((g_pTimer->TotalTime() - m_fTimeElapsed) >= 1.0f)
	{
		//float fps = 1 / g_pTimer->DeltaTime();
		float fps = (float)m_nFrameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << "Main"		   << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(g_hWnd, outs.str().c_str());

		// Reset for next average.
		m_nFrameCnt = 0;
		m_fTimeElapsed += 1.0f;
	}
}