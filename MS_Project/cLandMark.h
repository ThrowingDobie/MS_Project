#pragma once

#include "cWaves.h"

class cLandMark
{
public:
	cLandMark();
	~cLandMark();

	void Setup();
	void Render();
	void Update(float fDelta);

private:
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();

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
	XMFLOAT4X4 m_matWaterTexTransform;
	XMFLOAT4X4 m_matGrassTexTransform;
	XMFLOAT4X4 m_matLandWorld;
	XMFLOAT4X4 m_matWavesWorld;

	RenderOptions m_RenderOptions;
};

