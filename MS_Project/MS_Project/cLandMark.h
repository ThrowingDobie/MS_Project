#pragma once

#include "cWaves.h"

class cLandMark
{
public:
	cLandMark();
	~cLandMark();

	void Setup();
	void Init();
	void Render();
	void Update(float fDelta);

	void DrawTreeSprites();
	void TessellationRender();

private:
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildTreeSpritesBuffer();
	void BuildQuadPatchBuffer();

	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;

private:
	// Land
	UINT m_nLandIndexCount;
	ID3D11Buffer* m_pLandVertexBuffer;
	ID3D11Buffer* m_pLandIndexBuffer;
	Material m_mtLand;
	ID3D11ShaderResourceView* m_pGrassMapSRV;

private:
	// Waves
	cWaves m_Waves;
	ID3D11Buffer* m_pWavesVertexBuffer;
	ID3D11Buffer* m_pWavesIndexBuffer;
	Material m_mtWaves;
	ID3D11ShaderResourceView* m_pWavesMapSRV;

	XMFLOAT2 m_WaterTexOffset;

private:
	// Tree
	ID3D11Buffer* m_pTreeSpritesVB;
	Material m_mtTree;
	ID3D11ShaderResourceView* m_pTreeTextureMapArraySRV;

	static const UINT m_nTreeCount = 16;

	bool m_isAlphaToCoverageOn;

private:
	XMFLOAT4X4 m_matWaterTexTransform;
	XMFLOAT4X4 m_matGrassTexTransform;
	XMFLOAT4X4 m_matLandWorld;
	XMFLOAT4X4 m_matWavesWorld;

	RenderOptions m_RenderOptions;

	// Light
	DirectionalLight m_DirLights[3];

	// Tessellation
	ID3D11Buffer* m_pQuadPatchVertexBuffer;
};

