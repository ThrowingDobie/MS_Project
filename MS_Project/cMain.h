#pragma once

class cCube;
class cLandMark;

class cMain
{
private:
	cCube* m_pCube;
	cLandMark* m_pLandMark;

public:
	cMain();
	~cMain();

	void Setup();
	void LightSetup();
	void Update(float fDelta);
	void Render();

private:
	DirectionalLight m_DirLights[3];
};

