#pragma once

#define g_pD3DDevice cDeviceManager::GetInstance()

class cDeviceManager
{
	SINGLETONE(cDeviceManager);
public:
	void Setup();
	void Destroy();
	void OnResize();

	bool InitDevice();
	void CalculateFrameStats();

public:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDevCon;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Texture2D* m_pStencilBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11DepthStencilView* m_pDepthStencilView;
	D3D11_VIEWPORT m_ScreenViewport;

	int m_nClientWidth;
	int m_nClientHeight;

	bool m_isEnable4xMsaa;
	UINT m_n4xMsaaQuality;

	D3D_DRIVER_TYPE m_d3dDriverType;

	bool      mAppPaused;
	bool      mMinimized;
	bool      mMaximized;
	bool      mResizing;

	int m_nFrameCnt;
	float m_fTimeElapsed;
};

