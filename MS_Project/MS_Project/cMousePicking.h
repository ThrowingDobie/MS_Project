#pragma once

class cMousePicking
{
public:
	cMousePicking::cMousePicking();
	cMousePicking::~cMousePicking();

	void Setup();
	void Init(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, std::vector<float> vecHeight);
	void Update(float fDelta);
	void Render(DirectionalLight lights[3]);
	void OnMouseDown(WPARAM btnState, int nX, int nY);

	std::vector<Vertex::ST_P_VERTEX> GetHeight();

	bool HeightEdit();
private:
	void Pick(int nX, int nY);
	void BuildMeshGeometryBuffers();

	float GetGaussian(float fX, float fZ, float fRho);
	void CalGauss(int nX, int nZ, float fDelta);

	float m_fRho;

private:
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	std::vector<Vertex::Basic32> m_vecMeshVertices;
	std::vector<UINT> m_vecMeshIndices;
	XNA::AxisAlignedBox mMeshBox;

	XMFLOAT4X4 m_matMeshWorld;

	Material m_mtMesh;
	Material m_mtPickedTriangle;

	UINT m_nMeshIndexCount;
	UINT m_nPickedTriangle;

	std::vector<float> m_vecHeight;
	std::vector<Vertex::ST_P_VERTEX> m_vecVertex;

	XMFLOAT3 m_vPickingPoint;
};

