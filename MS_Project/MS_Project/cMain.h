#pragma once

class cCube;
class cLandMark;
class cSkull;

class cMain
{
private:
	cCube* m_pCube;
	cLandMark* m_pLandMark;
    cSkull* m_pSkull;

public:
	cMain();
	~cMain();

	void Setup();
    void Init();
	void Update(float fDelta);
	void Render();
};

