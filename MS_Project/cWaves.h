#pragma once

#ifndef WAVES_H
#define WAVES_H

class cWaves
{
public:
    cWaves();
    ~cWaves();

	UINT RowCount()const;
	UINT ColumnCount()const;
	UINT VertexCount()const;
	UINT TriangleCount()const;
    float Width()const;
    float Depth()const;

	// Returns the solution at the ith grid point.
	const XMFLOAT3& operator[](int i)const { return m_pvCurrSolution[i]; }

    // Returns the solution normal at the ith grid point.
    const XMFLOAT3& Normal(int i)const { return m_pvNormals[i]; }

    // Returns the unit tangent vector at the ith grid point in the local x-axis direction.
    const XMFLOAT3& TangentX(int i)const { return m_pvTangentX[i]; }

	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float fDelta);
	void Disturb(UINT i, UINT j, float magnitude);

public:

	UINT m_nNumRows;
	UINT m_nNumCols;

	UINT m_nVertexCount;
	UINT m_nTriangleCount;

	// Simulation constants we can precompute.
	float m_fK1;
	float m_fK2;
	float m_fK3;

	float m_fTimeStep;
	float m_fSpatialStep;

	XMFLOAT3* m_pvPrevSolution;
	XMFLOAT3* m_pvCurrSolution;
    XMFLOAT3* m_pvNormals;
    XMFLOAT3* m_pvTangentX;
};

#endif