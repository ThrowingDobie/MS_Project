#pragma once


#define g_pCamera cCamera::GetInstance()

class cCamera
{
	SINGLETONE(cCamera);

public:
	
	void Setup();
	void Update(float fDelta);
	void OnResize();

	void MouseDown(WPARAM btnState, int nX, int nY);
	void MouseUp(WPARAM btnState, int nX, int nY);
	void MouseMove(WPARAM btnState, int nX, int nY);

	XMFLOAT4X4 m_matWorld;
	XMFLOAT4X4 m_matView;
	XMFLOAT4X4 m_matProj;

	XMFLOAT3 m_vEyePosW;

private:

	float m_fTheta;
	float m_fPhi;
	float m_fRadius;

	POINT m_posLastMouse;

};

