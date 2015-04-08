#pragma once

class cCube;
class cLandMark;
class cSkull;
class cTerrain;

class cMain
{
private:
	cCube* m_pCube;
	cLandMark* m_pLandMark;
    cSkull* m_pSkull;
	cTerrain* m_pTerrain;

private:
	DirectionalLight m_DirLights[3];

private:
	void LightSetup();

public:
	cMain();
	~cMain();

	void Setup();
    void Init();
	void Update(float fDelta);
	void Render();
};

