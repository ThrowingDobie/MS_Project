#pragma once
class cCube
{
public:
	cCube();
	~cCube();

	void Setup();
	void Update(float fDelta);
	void Render();

private:

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	ID3D11ShaderResourceView* mBoxMapSRV;
	Material m_mtBox;

	ID3DX11Effect* m_pFX;
	ID3DX11EffectTechnique* m_pTech;
	ID3DX11EffectMatrixVariable* m_pfxWorldViewProj;

	ID3D11InputLayout* m_pInputLayout;

	XMFLOAT4X4 m_matBoxWorld;

	RenderOptions m_RenderOptions;

	void BuildCrateGeometryBuffers();
};

