#pragma once

class cQuadTree
{
public:
	cQuadTree();
	cQuadTree(int nX, int nY);
	cQuadTree(cQuadTree* pNode);
	~cQuadTree();

	// MS
	bool AddChild();
	void Destroy();

	bool GetIsVisible();

	std::vector<UINT> GetIndex();
	std::vector<cQuadTree*> m_vecChild;

	int	   m_nCenter;			// 쿼드트리 인자1
private:
	int    m_nCorner[4];		// 쿼드트리 인자2
private:

	// 4개의 코너값을 셋팅
	void SetCorners(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);
};

