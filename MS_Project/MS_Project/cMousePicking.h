#pragma once

class cQuadTree;
class cOctree;

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
	std::vector<D3DXVECTOR3> GetVecPoint();
	DirectX::TextureType GetTextureType();

	bool HeightEdit();
	bool TextureMap();
private:
	void Pick(int nX, int nY);
	void BuildMeshGeometryBuffers();

	float GetGaussian(float fX, float fZ, float fRho);
	void CalGauss(int nX, int nZ, float fDelta);

    void SelectCircle(int nX, int nZ, int nRange);
    void EraseHeight(int nRange);

    void KeyUpdate(bool isUpdate);

	float m_fRho;
	TerrainEditType m_eEditType;

    KeyType m_eKeyTest;
    KeyType m_eKeyReturn;

	XMVECTOR GetNearPoint(std::vector<XMVECTOR> vecPoint);

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

	bool m_isRightClick;

	DirectX::ST_PD_VERTEX m_pdVertex;
	std::vector<D3DXVECTOR3> m_vecPoint;
	std::vector<float> m_vecDepth;
	DirectX::TextureType m_eTextureType;

	ID3D11Buffer* m_pQuadPatchIndexBuffer;
	void SetMappingData();
public:
	DirectX::ST_PD_VERTEX GetMappingData();

private:
	// QuadTree
	cQuadTree* m_pQuadTree;

	bool CalTail(cQuadTree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist);

	cQuadTree* m_pNode;

	std::vector<XMVECTOR> m_vecTest;
	std::vector<cQuadTree*> m_vecQuad;
	std::vector<float> m_vecDist;

	// Octree
	cOctree* m_pOctree;

	std::vector<cOctree*> m_vecOctree;

	void CalPoint(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist);

	XMFLOAT3 SetIndex(int nIndex, int nSize);

	std::vector<XMFLOAT3> m_vecSavePoint;

};

