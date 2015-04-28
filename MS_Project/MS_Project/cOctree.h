#pragma once

class cOctree
{
public:
	cOctree();
	cOctree(int nSize);
	~cOctree();

	bool AddChild();
	void Destroy();
	bool GetIsVisible();

	std::vector<cOctree*> GetChild();
	std::vector<int> GetCorner();
	int GetCenter();

	std::vector<UINT> GetIndex();

	XMFLOAT3 GetCenterVector();

private:
	std::vector<cOctree*> m_vecChild;
	std::vector<int> m_vecCorner;
	int m_nCenter;

	XMFLOAT3 m_vCenter;

private:
	void SetCorner(std::vector<int> vecCorner);
	XMFLOAT3 SetIndex(int nIndex, int nSize);
};

