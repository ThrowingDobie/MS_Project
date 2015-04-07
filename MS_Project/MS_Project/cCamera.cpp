#include "stdafx.h"
#include "cCamera.h"

cCamera::cCamera()
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_matWorld, I);
	XMStoreFloat4x4(&m_matView, I);
	XMStoreFloat4x4(&m_matProj, I);
	
	m_fTheta = 1.5f*D3DX_PI;
	m_fPhi = 0.25f*D3DX_PI;
	m_fRadius = 80.f;

	m_posLastMouse = { 0, 0 };
}


cCamera::~cCamera()
{
}

void cCamera::Setup()
{

}

void cCamera::Update(float fDelta)
{
	float x = m_fRadius*sinf(m_fPhi)*cosf(m_fTheta);
	float z = m_fRadius*sinf(m_fPhi)*sinf(m_fTheta);
	float y = m_fRadius*cosf(m_fPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_vEyePosW = XMFLOAT3(x, y, z);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_matView, V);
}

void cCamera::MouseDown(WPARAM btnState, int nX, int nY)
{
	m_posLastMouse.x = nX;
	m_posLastMouse.y = nY;

	SetCapture(g_hWnd);
}

void cCamera::MouseUp(WPARAM btnState, int nX, int nY)
{
	ReleaseCapture();
}

void cCamera::MouseMove(WPARAM btnState, int nX, int nY)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(nX - m_posLastMouse.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(nY - m_posLastMouse.y));

		// Update angles based on input to orbit camera around box.
		m_fTheta += dx;
		m_fPhi += dy;

		// Restrict the angle mPhi.
		m_fPhi = MathHelper::Clamp(m_fPhi, 0.1f, (float)D3DX_PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.1f*static_cast<float>(nX - m_posLastMouse.x);
		float dy = 0.1f*static_cast<float>(nY - m_posLastMouse.y);

		// Update the camera radius based on input.
		m_fRadius += dx - dy;

		// Restrict the radius.
		m_fRadius = MathHelper::Clamp(m_fRadius, 3.0f, 300.0f);
	}

	m_posLastMouse.x = nX;
	m_posLastMouse.y = nY;
}

void cCamera::OnResize()
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*D3DX_PI,
		g_pD3DDevice->m_nClientWidth/g_pD3DDevice->m_nClientHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_matProj, P);
}