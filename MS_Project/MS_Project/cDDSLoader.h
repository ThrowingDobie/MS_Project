#pragma once

#define g_pDDSLoader cDDSLoader::GetInstance()

#define MAX_MIPMAP_LEVEL 16

class cDDSLoader
{
    SINGLETONE(cDDSLoader);

public:
    void Setup();
    void Init();
    void Update(float fDelta);
    void Render();


private:
	BYTE* m_cpPixels[MAX_MIPMAP_LEVEL];
	int m_nBytes[MAX_MIPMAP_LEVEL];

	int m_nWidth;
	int m_nHeight;
	int m_nMipmapLevel;
	int m_nFormat;
	int m_nInternalFormat;
	int m_nCompressionType;

	int GetWidth(){ return m_nWidth; }
	int GetHeight(){ return m_nHeight; }
	int GetMipMapLevels(){ return m_nMipmapLevel; }

	bool Load(std::string filename);

	DTexture *CreateTexture();
};

