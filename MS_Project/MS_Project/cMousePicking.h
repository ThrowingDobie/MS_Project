#pragma once

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
	bool GetIsMousePosChange();

	void SetMouseRbutton(bool isClick);
	bool GetMouseRbutton();

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

private:
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	std::vector<Vertex::Basic32> m_vecMeshVertices;
	std::vector<UINT> m_vecMeshIndices;
	XNA::AxisAlignedBox mMeshBox;

	XMFLOAT4X4 m_matMeshWorld;
	XMFLOAT4X4 m_matScale;

	//Material m_mtMesh;
	//Material m_mtPickedTriangle;

	UINT m_nMeshIndexCount;
	UINT m_nPickedTriangle;

	std::vector<float> m_vecHeight;
	std::vector<Vertex::ST_P_VERTEX> m_vecVertex;

    std::vector<Vertex::ST_P_VERTEX> m_vecSave;
    std::stack<std::vector<Vertex::ST_P_VERTEX>> m_StackPrve;
    std::stack<std::vector<Vertex::ST_P_VERTEX>> m_StackNext;

	XMFLOAT3 m_vPickingPoint;

	bool m_isRightClick;

	// TextureMapping

	DirectX::ST_PD_VERTEX m_pdVertex;
	std::vector<D3DXVECTOR3> m_vecPoint;
	std::vector<float> m_vecDepth;
	DirectX::TextureType m_eTextureType;
	DirectX::TextureUsingType m_eTextureUsingType;

	bool m_isChangeMappingData;

	// MouseMapping

	DirectX::ST_PD_VERTEX m_pdVertex_Mouse;
	std::vector<D3DXVECTOR3> m_vecPoint_Mouse;
	std::vector<float> m_vecDepth_Mouse;
	DirectX::TextureType m_eTextureType_Mouse;
	DirectX::TextureUsingType m_eTextureUsingType_Mouse;


	ID3D11Buffer* m_pQuadPatchIndexBuffer;
	int m_nMapSize;

public:
	DirectX::ST_PD_VERTEX GetMappingData();
	DirectX::ST_PD_VERTEX GetMouseMappingData();
	void SetMappingData();
	void SetMouseMappingData();

	bool GetIsChangeMappingData();

private:
	XMVECTOR GetNearPoint(std::vector<XMVECTOR> vecPoint);
	XMVECTOR GetNearPointTest(std::vector<XMFLOAT3> vecPoint);

	//// QuadTree
	//cQuadTree* m_pQuadTree;
	//bool CalTail(cQuadTree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist);
	//std::vector<XMVECTOR> m_vecTest;

	// Octree
	cOctree* m_pOctree;
	std::vector<cOctree*> m_vecOctree;
	void CalPoint(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist);
	XMFLOAT3 GetPosByIndex(int nIndex, int nSize);
	bool SelectTile(cOctree* pRoot, XMVECTOR vOrigin, XMVECTOR vDir, float fDist);
	bool CulDataPicking(int nIndexFirst, int nRange, XMVECTOR vOrigin, XMVECTOR vDir);

	std::vector<XMVECTOR> m_vecColliedTri;
	std::vector<XMFLOAT3> m_vecColliedTri3;
};

