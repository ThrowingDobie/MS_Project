#pragma once
class cTerrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;
		std::wstring MousePointFilename;
		std::wstring MouseBlendFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};


public:
	cTerrain();
	~cTerrain();

	void Setup();
	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);
	void Update(float fDelta);
	void Render(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);

	float GetHorizon()const;
	float GetVertical()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void ChangeHeightData(std::vector<Vertex::ST_P_VERTEX> vecVertex);

	float Average(int i, int j);
private:
	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightmapSRV(ID3D11Device* device);


private:
	static const int CellsPerPatch = 64;

	ID3D11ShaderResourceView* m_pLayerMapArraySRV;
	ID3D11ShaderResourceView* m_pBlendMapSRV;
	ID3D11ShaderResourceView* m_pHeightMapSRV;
	ID3D11ShaderResourceView* m_pMouseBlendSRV;

	InitInfo m_Info;

	UINT m_nPatchVertices;
	UINT m_nPatchQuadFaces;

	UINT m_nPatchVertRows;
	UINT m_nPatchVertCols;

	XMFLOAT4X4 m_matWorld;
	XMFLOAT4X4 m_matScale;
	XMFLOAT4X4 m_matRot;
    XMFLOAT4X4 m_matTrs;

	Material m_mtTerrain;

	std::vector<XMFLOAT2> m_vecPatchBoundsY;

public:

	ID3D11Buffer* m_pQuadPatchVertexBuffer;
	ID3D11Buffer* m_pQuadPatchIndexBuffer;

	std::vector<float> m_vecHeightmap;

	DirectX::ST_PD_VERTEX m_pdVertex;
	DirectX::ST_PD_VERTEX m_pdVertex_Mouse;

	void SetMappingData(DirectX::ST_PD_VERTEX pdVertex);
	void SetMouseMappingData(DirectX::ST_PD_VERTEX pdVertex);
public:
	DirectX::ST_PD_VERTEX GetMappingData();
};

