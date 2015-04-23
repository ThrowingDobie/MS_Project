#include "stdafx.h"
#include "cOctree.h"


cOctree::cOctree()
{
	m_nCenter = 0;
	m_vecChild.resize(8);
	m_vecCorner.resize(8);
}

cOctree::cOctree(int nSize)
{
	m_nCenter = 0;
	m_vecChild.resize(8);
	m_vecCorner.resize(8);
	
	m_vecCorner[0] = 0;
	m_vecCorner[1] = nSize - 1;
	m_vecCorner[2] = nSize * (nSize - 1);
	m_vecCorner[3] = nSize * nSize - 1;
	m_vecCorner[4] = m_vecCorner[0] + nSize*nSize*(nSize - 1);
	m_vecCorner[5] = m_vecCorner[1] + nSize*nSize*(nSize - 1);
	m_vecCorner[6] = m_vecCorner[2] + nSize*nSize*(nSize - 1);
	m_vecCorner[7] = m_vecCorner[3] + nSize*nSize*(nSize - 1);
}

cOctree::~cOctree()
{
	Destroy();
}

void cOctree::Destroy()
{
	for (auto p : m_vecChild)
	{
		if (p)
		{
			p->Destroy();
		}
	}
}

bool cOctree::AddChild()
{
	std::vector<int> vecCorner;
	vecCorner.resize(27);

	int nX = (m_vecCorner[1] - m_vecCorner[0]) / 2;
	int nY = (m_vecCorner[4] - m_vecCorner[0]) / 2;
	int nZ = (m_vecCorner[2] - m_vecCorner[0]) / 2;

	vecCorner[0] = m_vecCorner[0];
	vecCorner[1] = vecCorner[0] + nX;
	vecCorner[2] = vecCorner[1] + nX;

	vecCorner[3] = vecCorner[0] + nZ;
	vecCorner[4] = vecCorner[3] + nX;
	vecCorner[5] = vecCorner[4] + nX;
	
	vecCorner[6] = vecCorner[3] + nZ;
	vecCorner[7] = vecCorner[6] + nX;
	vecCorner[8] = vecCorner[7] + nX;
	
	vecCorner[9] = vecCorner[0] + nY;
	vecCorner[10] = vecCorner[9] + nX;
	vecCorner[11] = vecCorner[10] + nX;
	
	vecCorner[12] = vecCorner[9] + nZ;
	vecCorner[13] = vecCorner[12] + nX;
	vecCorner[14] = vecCorner[13] + nX;
	
	vecCorner[15] = vecCorner[12] + nZ;
	vecCorner[16] = vecCorner[15] + nX;
	vecCorner[17] = vecCorner[16] + nX;
	
	vecCorner[18] = vecCorner[9] + nY;
	vecCorner[19] = vecCorner[18] + nX;
	vecCorner[20] = vecCorner[19] + nX;
	
	vecCorner[21] = vecCorner[18] + nZ;
	vecCorner[22] = vecCorner[21] + nX;
	vecCorner[23] = vecCorner[22] + nX;

	vecCorner[24] = vecCorner[21] + nZ;
	vecCorner[25] = vecCorner[24] + nX;
	vecCorner[26] = vecCorner[25] + nX;

	if (GetIsVisible())
	{
		return false;
	}
	else
	{
		std::vector<int> inputCorner0;
		int inputCenter0 = 0;
		inputCorner0.resize(8);
		cOctree* pChild0 = new cOctree;
		inputCorner0[0] = vecCorner[0];
		inputCorner0[1] = vecCorner[1];
		inputCorner0[2] = vecCorner[3];
		inputCorner0[3] = vecCorner[4];
		inputCorner0[4] = vecCorner[9];
		inputCorner0[5] = vecCorner[10];
		inputCorner0[6] = vecCorner[12];
		inputCorner0[7] = vecCorner[13];
		inputCenter0 = ((inputCorner0[0] + inputCorner0[1] + inputCorner0[2] + inputCorner0[3]) / 4)
			+ ((inputCorner0[4] - inputCorner0[0]) / 2);
		pChild0->m_vecCorner = inputCorner0;
		pChild0->m_nCenter = inputCenter0;
		pChild0->m_vCenter = SetIndex(pChild0->m_nCenter, 257);
		m_vecChild[0] = pChild0;

		std::vector<int> inputCorner1;
		int inputCenter1 = 0;
		inputCorner1.resize(8);
		cOctree* pChild1 = new cOctree;
		inputCorner1[0] = vecCorner[1];
		inputCorner1[1] = vecCorner[2];
		inputCorner1[2] = vecCorner[4];
		inputCorner1[3] = vecCorner[5];
		inputCorner1[4] = vecCorner[10];
		inputCorner1[5] = vecCorner[11];
		inputCorner1[6] = vecCorner[13];
		inputCorner1[7] = vecCorner[14];
		inputCenter1 = ((inputCorner1[0] + inputCorner1[1] + inputCorner1[2] + inputCorner1[3]) / 4)
			+ ((inputCorner1[4] - inputCorner1[0]) / 2);
		pChild1->m_vecCorner = inputCorner1;
		pChild1->m_nCenter = inputCenter1;
		pChild1->m_vCenter = SetIndex(pChild1->m_nCenter, 257);
		m_vecChild[1] = pChild1;

		std::vector<int> inputCorner2;
		int inputCenter2 = 0;
		inputCorner2.resize(8);
		cOctree* pChild2 = new cOctree;
		inputCorner2[0] = vecCorner[3];
		inputCorner2[1] = vecCorner[4];
		inputCorner2[2] = vecCorner[6];
		inputCorner2[3] = vecCorner[7];
		inputCorner2[4] = vecCorner[12];
		inputCorner2[5] = vecCorner[13];
		inputCorner2[6] = vecCorner[15];
		inputCorner2[7] = vecCorner[16];
		inputCenter2 = ((inputCorner2[0] + inputCorner2[1] + inputCorner2[2] + inputCorner2[3]) / 4)
			+ ((inputCorner2[4] - inputCorner2[0]) / 2);
		pChild2->m_vecCorner = inputCorner2;
		pChild2->m_nCenter = inputCenter2;
		pChild2->m_vCenter = SetIndex(pChild2->m_nCenter, 257);
		m_vecChild[2] = pChild2;

		std::vector<int> inputCorner3;
		int inputCenter3 = 0;
		inputCorner3.resize(8);
		cOctree* pChild3 = new cOctree;
		inputCorner3[0] = vecCorner[4];
		inputCorner3[1] = vecCorner[5];
		inputCorner3[2] = vecCorner[7];
		inputCorner3[3] = vecCorner[8];
		inputCorner3[4] = vecCorner[13];
		inputCorner3[5] = vecCorner[14];
		inputCorner3[6] = vecCorner[16];
		inputCorner3[7] = vecCorner[17];
		inputCenter3 = ((inputCorner3[0] + inputCorner3[1] + inputCorner3[2] + inputCorner3[3]) / 4)
			+ ((inputCorner3[4] - inputCorner3[0]) / 2);
		pChild3->m_vecCorner = inputCorner3;
		pChild3->m_nCenter = inputCenter3;
		pChild3->m_vCenter = SetIndex(pChild3->m_nCenter, 257);
		m_vecChild[3] = pChild3;

		std::vector<int> inputCorner4;
		int inputCenter4 = 0;
		inputCorner4.resize(8);
		cOctree* pChild4 = new cOctree;
		inputCorner4[0] = vecCorner[9];
		inputCorner4[1] = vecCorner[10];
		inputCorner4[2] = vecCorner[12];
		inputCorner4[3] = vecCorner[13];
		inputCorner4[4] = vecCorner[18];
		inputCorner4[5] = vecCorner[19];
		inputCorner4[6] = vecCorner[21];
		inputCorner4[7] = vecCorner[22];
		inputCenter4 = ((inputCorner4[0] + inputCorner4[1] + inputCorner4[2] + inputCorner4[3]) / 4)
			+ ((inputCorner4[4] - inputCorner4[0]) / 2);
		pChild4->m_vecCorner = inputCorner4;
		pChild4->m_nCenter = inputCenter4;
		pChild4->m_vCenter = SetIndex(pChild4->m_nCenter, 257);
		m_vecChild[4] = pChild4;

		std::vector<int> inputCorner5;
		int inputCenter5 = 0;
		inputCorner5.resize(8);
		cOctree* pChild5 = new cOctree;
		inputCorner5[0] = vecCorner[10];
		inputCorner5[1] = vecCorner[11];
		inputCorner5[2] = vecCorner[13];
		inputCorner5[3] = vecCorner[14];
		inputCorner5[4] = vecCorner[19];
		inputCorner5[5] = vecCorner[20];
		inputCorner5[6] = vecCorner[22];
		inputCorner5[7] = vecCorner[23];
		inputCenter5 = ((inputCorner5[0] + inputCorner5[1] + inputCorner5[2] + inputCorner5[3]) / 4)
			+ ((inputCorner5[4] - inputCorner5[0]) / 2);
		pChild5->m_vecCorner = inputCorner5;
		pChild5->m_nCenter = inputCenter5;
		pChild5->m_vCenter = SetIndex(pChild5->m_nCenter, 257);
		m_vecChild[5] = pChild5;

		std::vector<int> inputCorner6;
		int inputCenter6 = 0;
		inputCorner6.resize(8);
		cOctree* pChild6 = new cOctree;
		inputCorner6[0] = vecCorner[12];
		inputCorner6[1] = vecCorner[13];
		inputCorner6[2] = vecCorner[15];
		inputCorner6[3] = vecCorner[16];
		inputCorner6[4] = vecCorner[21];
		inputCorner6[5] = vecCorner[22];
		inputCorner6[6] = vecCorner[24];
		inputCorner6[7] = vecCorner[25];
		inputCenter6 = ((inputCorner6[0] + inputCorner6[1] + inputCorner6[2] + inputCorner6[3]) / 4)
			+ ((inputCorner6[4] - inputCorner6[0]) / 2);
		pChild6->m_vecCorner = inputCorner6;
		pChild6->m_nCenter = inputCenter6;
		pChild6->m_vCenter = SetIndex(pChild6->m_nCenter, 257);
		m_vecChild[6] = pChild6;

		std::vector<int> inputCorner7;
		int inputCenter7 = 0;
		inputCorner7.resize(8);
		cOctree* pChild7 = new cOctree;
		inputCorner7[0] = vecCorner[13];
		inputCorner7[1] = vecCorner[14];
		inputCorner7[2] = vecCorner[16];
		inputCorner7[3] = vecCorner[17];
		inputCorner7[4] = vecCorner[22];
		inputCorner7[5] = vecCorner[23];
		inputCorner7[6] = vecCorner[25];
		inputCorner7[7] = vecCorner[26];
		inputCenter7 = ((inputCorner7[0] + inputCorner7[1] + inputCorner7[2] + inputCorner7[3]) / 4)
			+ ((inputCorner7[4] - inputCorner7[0]) / 2);
		pChild7->m_vecCorner = inputCorner7;
		pChild7->m_nCenter = inputCenter7;
		pChild7->m_vCenter = SetIndex(pChild7->m_nCenter, 257);
		m_vecChild[7] = pChild7;

		return true;
	}
}

bool cOctree::GetIsVisible()
{
	if (m_vecCorner[1] - m_vecCorner[0] <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<cOctree*> cOctree::GetChild()
{
	return m_vecChild;
}

std::vector<int> cOctree::GetCorner()
{
	return m_vecCorner;
}

int cOctree::GetCenter()
{
	return m_nCenter;
}

std::vector<UINT> cOctree::GetIndex()
{
	std::vector<UINT> vecIndex;
	for (int i = 0; i < 8; i++)
	{
		vecIndex.push_back((UINT)m_vecCorner[i]);
	}
	return vecIndex;
}

XMFLOAT3 cOctree::SetIndex(int nIndex, int nSize)
{
	XMFLOAT3 vPos;

	int nValue = nIndex / (nSize*nSize);
	int nMod = nIndex % (nSize*nSize);

	vPos.x = nMod % nSize;
	vPos.y = nValue;
	vPos.z = nMod / nSize;

	return vPos;
}