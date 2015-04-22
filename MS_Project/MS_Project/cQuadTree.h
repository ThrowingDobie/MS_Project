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
	void AddChild();

private:
	//cQuadTree* m_pChild[4];		// �ڽ� 4����

	int	   m_nCenter;			// ����Ʈ�� ����1
	int    m_nCorner[4];		// ����Ʈ�� ����2

	// MS
	std::vector<cQuadTree*> m_vecChild;

private:
	void Destroy();
	
	// ��� ������ ����ΰ�?
	bool GetIsVisible();

	// 4���� �ڳʰ��� ����
	void SetCorners(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// �ڽĳ�带 �߰�
	//cQuadTree* AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// ����Ʈ���� 4���� ���� Ʈ���� ��������
	bool SubDivide();


};

