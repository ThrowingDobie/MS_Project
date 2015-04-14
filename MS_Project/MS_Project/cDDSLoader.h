#pragma once

#define g_pDDSLoader cDDSLoader::GetInstance()

class cDDSLoader
{
    SINGLETONE(cDDSLoader);

public:
    //cDDSLoader();
    //~cDDSLoader();

    void Setup();
    void Init();
    void Update(float fDelta);
    void Render();
};

