#pragma once
class cQuadTree
{
public:
	cQuadTree();
	cQuadTree(int nX, int nY);
	cQuadTree(cQuadTree* pNode);
	~cQuadTree();

	bool Build();

	// MS
	bool AddChild();
	void Destroy();

	std::vector<UINT> GetIndex();
	std::vector<cQuadTree*> m_vecChild;

	int	   m_nCenter;			// 쿼드트리 인자1
private:

	int    m_nCorner[4];		// 쿼드트리 인자2
private:

	// 출력 가능한 노드인가?
	bool GetIsVisible();

	// 4개의 코너값을 셋팅
	void SetCorners(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// 자식노드를 추가
	//cQuadTree* AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// 쿼드트리를 4개의 하위 트리로 하위분할
	bool SubDivide();


};

