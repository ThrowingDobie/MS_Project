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

	int	   m_nCenter;			// ����Ʈ�� ����1
private:

	int    m_nCorner[4];		// ����Ʈ�� ����2
private:

	// ��� ������ ����ΰ�?
	bool GetIsVisible();

	// 4���� �ڳʰ��� ����
	void SetCorners(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// �ڽĳ�带 �߰�
	//cQuadTree* AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// ����Ʈ���� 4���� ���� Ʈ���� ��������
	bool SubDivide();


};

