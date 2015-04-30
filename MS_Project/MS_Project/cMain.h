#pragma once

class cCube;
class cLandMark;
class cSkull;
class cTerrain;
class cMousePicking;

class cMain
{
private:
	cCube* m_pCube;
	cLandMark* m_pLandMark;
    cSkull* m_pSkull;
	cTerrain* m_pTerrain;
	cMousePicking* m_pMouse;

private:
	DirectionalLight m_DirLights[3];

	float m_fAccumTime;

private:
	void LightSetup();

public:
	cMain();
	~cMain();

	void Setup();
    void Init();
	void Update(float fDelta);
	void Render();

	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

