#include "stdafx.h"
#include "cQuadTree.h"

cQuadTree::cQuadTree()
	: m_vecChild(NULL)
{
	m_nCenter = 0;
	for (int i = 0; i < 4; i++)
	{
		m_nCorner[i] = 0;
	}
}

cQuadTree::cQuadTree(int nX, int nY)
{
	m_nCenter = 0;

	m_nCorner[E_CornerTL] = 0;
	m_nCorner[E_CornerTR] = nX - 1;
	m_nCorner[E_CornerBL] = nX * (nY - 1);
	m_nCorner[E_CornerBR] = nX * nY - 1;

	m_nCenter = (m_nCorner[E_CornerTL] + m_nCorner[E_CornerTR]
		+ m_nCorner[E_CornerBL] + m_nCorner[E_CornerBR]) / 4;
}


cQuadTree::~cQuadTree()
{
	Destroy();
}

void cQuadTree::Destroy()
{
	for (auto p : m_vecChild)
	{
		if (p->m_vecChild.size() > 0)
		{
			p->Destroy();
		}
	}
}

bool cQuadTree::AddChild()
{
	int nTopEdgeCenter = 0;
	int nBottomEdgeCenter = 0;
	int nLeftEdgCenter = 0;
	int nRightEdgeCenter = 0;
	int nCentralPoint = 0;

	// »ó
	nTopEdgeCenter = (m_nCorner[E_CornerTL] + m_nCorner[E_CornerTR]) / 2;
	// ÇÏ
	nBottomEdgeCenter = (m_nCorner[E_CornerBL] + m_nCorner[E_CornerBR]) / 2;
	// ÁÂ
	nLeftEdgCenter = (m_nCorner[E_CornerTL] + m_nCorner[E_CornerBL]) / 2;
	// ¿ì
	nRightEdgeCenter = (m_nCorner[E_CornerTR] + m_nCorner[E_CornerBR]) / 2;
	// Áß¾Ó
	nCentralPoint = (m_nCorner[E_CornerTL] + m_nCorner[E_CornerTR]
		+ m_nCorner[E_CornerBL] + m_nCorner[E_CornerBR]) / 4;

	if (GetIsVisible())
	{
		return false;
	}
	else
	{
		m_vecChild.resize(4);

		cQuadTree* pChildTL = new cQuadTree;
		pChildTL->SetCorners(m_nCorner[E_CornerTL], nTopEdgeCenter,
			nLeftEdgCenter, nCentralPoint);
		m_vecChild[0] = pChildTL;

		cQuadTree* pChildTR = new cQuadTree;
		pChildTR->SetCorners(nTopEdgeCenter, m_nCorner[E_CornerTR],
			nCentralPoint, nRightEdgeCenter);
		m_vecChild[1] = pChildTR;

		cQuadTree* pChildBL = new cQuadTree;
		pChildBL->SetCorners(nLeftEdgCenter, nCentralPoint,
			m_nCorner[E_CornerBL], nBottomEdgeCenter);
		m_vecChild[2] = pChildBL;

		cQuadTree* pChildBR = new cQuadTree;
		pChildBR->SetCorners(nCentralPoint, nRightEdgeCenter,
			nBottomEdgeCenter, m_nCorner[E_CornerBR]);
		m_vecChild[3] = pChildBR;

		return true;
	}
}

std::vector<UINT> cQuadTree::GetIndex()
{
	std::vector<UINT> vecIndex;
	for (int i = 0; i < 4; i++)
	{
		vecIndex.push_back((UINT)m_nCorner[i]);
	}
	return vecIndex;
}

void cQuadTree::SetCorners(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR)
{
	m_nCorner[E_CornerTL] = nCornerTL;
	m_nCorner[E_CornerTR] = nCornerTR;
	m_nCorner[E_CornerBL] = nCornerBL;
	m_nCorner[E_CornerBR] = nCornerBR;
	m_nCenter = (m_nCorner[E_CornerTL] + m_nCorner[E_CornerTR]
		+ m_nCorner[E_CornerBL] + m_nCorner[E_CornerBR]) / 4;
}
//
bool cQuadTree::GetIsVisible()
{
	if (m_nCorner[E_CornerTR] - m_nCorner[E_CornerTL] <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//}