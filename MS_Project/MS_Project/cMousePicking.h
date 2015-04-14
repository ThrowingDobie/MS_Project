#pragma once

class cMousePicking
{
	enum TerrainEditType
	{
		E_INCREASE,
		E_DECREASE,
        E_NORMALIZE,
		E_ERASE,
		E_EMPTY,
        E_TEST,
		E_MAX
	};

    enum KeyType
    {
        E_KEYDOWN,
        E_DOWN,
        E_KEYUP,
        E_UP,
    };
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

    std::vector<D3DXVECTOR3> SelectCircle(int nX, int nZ, int nRange);
    void EraseHeight(int nRange);

    void KeyUpdate(bool isUpdate);

	float m_fRho;
	TerrainEditType m_eEditType;

    KeyType m_eKeyTest;
    KeyType m_eKeyReturn;

private:
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	std::vector<Vertex::Basic32> m_vecMeshVertices;
	std::vector<UINT> m_vecMeshIndices;
	XNA::AxisAlignedBox mMeshBox;

	XMFLOAT4X4 m_matMeshWorld;
	XMFLOAT4X4 m_matScale;

	Material m_mtMesh;
	Material m_mtPickedTriangle;

	UINT m_nMeshIndexCount;
	UINT m_nPickedTriangle;

	std::vector<float> m_vecHeight;
	std::vector<Vertex::ST_P_VERTEX> m_vecVertex;

    std::vector<Vertex::ST_P_VERTEX> m_vecSave;
    std::stack<std::vector<Vertex::ST_P_VERTEX>> m_StackPrve;
    std::stack<std::vector<Vertex::ST_P_VERTEX>> m_StackNext;

	XMFLOAT3 m_vPickingPoint;
};

