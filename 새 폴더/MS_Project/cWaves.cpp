#include "stdafx.h"
#include "cWaves.h"


cWaves::cWaves()
	: m_nNumRows(0)
	, m_nNumCols(0)
	, m_nVertexCount(0)
	, m_nTriangleCount(0)
	, m_fK1(0.f)
	, m_fK2(0.f)
	, m_fK3(0.f)
	, m_fTimeStep(0.f)
	, m_fSpatialStep(0.f)
	, m_pvPrevSolution(0)
	, m_pvCurrSolution(0)
    , m_pvNormals(0)
    , m_pvTangentX(0)
{
}

cWaves::~cWaves()
{
	delete[] m_pvPrevSolution;
	delete[] m_pvCurrSolution;
    delete[] m_pvNormals;
    delete[] m_pvTangentX;
}

UINT cWaves::RowCount()const
{
	return m_nNumRows;
}

UINT cWaves::ColumnCount()const
{
	return m_nNumCols;
}

UINT cWaves::VertexCount()const
{
	return m_nVertexCount;
}

UINT cWaves::TriangleCount()const
{
	return m_nTriangleCount;
}

float cWaves::Width()const
{
    return m_nNumCols*m_fSpatialStep;
}

float cWaves::Depth()const
{
    return m_nNumRows*m_fSpatialStep;
}

void cWaves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	m_nNumRows = m;
	m_nNumCols = n;

	m_nVertexCount = m*n;
	m_nTriangleCount = (m - 1)*(n - 1) * 2;

	m_fTimeStep = dt;
	m_fSpatialStep = dx;

	float d = damping*dt + 2.0f;
	float e = (speed*speed)*(dt*dt) / (dx*dx);
	m_fK1 = (damping*dt - 2.0f) / d;
	m_fK2 = (4.0f - 8.0f*e) / d;
	m_fK3 = (2.0f*e) / d;

	// In case Init() called again.
	delete[] m_pvPrevSolution;
	delete[] m_pvCurrSolution;
    delete[] m_pvNormals;
    delete[] m_pvTangentX;

	m_pvPrevSolution = new XMFLOAT3[m*n];
	m_pvCurrSolution = new XMFLOAT3[m*n];
    m_pvNormals      = new XMFLOAT3[m*n];
    m_pvTangentX     = new XMFLOAT3[m*n];

	// Generate grid vertices in system memory.

	float halfWidth = (n - 1)*dx*0.5f;
	float halfDepth = (m - 1)*dx*0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;

			m_pvPrevSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);
			m_pvCurrSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);
            m_pvNormals[i*n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_pvTangentX[i*n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}
}

void cWaves::Update(float dt)
{
    static float t = 0;

    // Accumulate time.
    t += dt;

    // Only update the simulation at the specified time step.
    if (t >= m_fTimeStep)
    {
        // Only update interior points; we use zero boundary conditions.
        for (UINT i = 1; i < m_nNumRows - 1; ++i)
        {
            for (UINT j = 1; j < m_nNumCols - 1; ++j)
            {
                // After this update we will be discarding the old previous
                // buffer, so overwrite that buffer with the new update.
                // Note how we can do this inplace (read/write to same element) 
                // because we won't need prev_ij again and the assignment happens last.

                // Note j indexes x and i indexes z: h(x_j, z_i, t_k)
                // Moreover, our +z axis goes "down"; this is just to 
                // keep consistent with our row indices going down.

                m_pvPrevSolution[i*m_nNumCols + j].y =
                    m_fK1*m_pvPrevSolution[i*m_nNumCols + j].y +
                    m_fK2*m_pvCurrSolution[i*m_nNumCols + j].y +
                    m_fK3*(m_pvCurrSolution[(i + 1)*m_nNumCols + j].y +
                    m_pvCurrSolution[(i - 1)*m_nNumCols + j].y +
                    m_pvCurrSolution[i*m_nNumCols + j + 1].y +
                    m_pvCurrSolution[i*m_nNumCols + j - 1].y);
            }
        }

        // We just overwrote the previous buffer with the new data, so
        // this data needs to become the current solution and the old
        // current solution becomes the new previous solution.
        std::swap(m_pvPrevSolution, m_pvCurrSolution);

        t = 0.0f; // reset time

        //
        // Compute normals using finite difference scheme.
        //
        for (UINT i = 1; i < m_nNumRows - 1; ++i)
        {
            for (UINT j = 1; j < m_nNumCols - 1; ++j)
            {
                float l = m_pvCurrSolution[i*m_nNumCols + j - 1].y;
                float r = m_pvCurrSolution[i*m_nNumCols + j + 1].y;
                float t = m_pvCurrSolution[(i - 1)*m_nNumCols + j].y;
                float b = m_pvCurrSolution[(i + 1)*m_nNumCols + j].y;
                m_pvNormals[i*m_nNumCols + j].x = -r + l;
                m_pvNormals[i*m_nNumCols + j].y = 2.0f*m_fSpatialStep;
                m_pvNormals[i*m_nNumCols + j].z = b - t;

                XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_pvNormals[i*m_nNumCols + j]));
                XMStoreFloat3(&m_pvNormals[i*m_nNumCols + j], n);

                m_pvTangentX[i*m_nNumCols + j] = XMFLOAT3(2.0f*m_fSpatialStep, r - l, 0.0f);
                XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_pvTangentX[i*m_nNumCols + j]));
                XMStoreFloat3(&m_pvTangentX[i*m_nNumCols + j], T);
            }
        }
    }
}

void cWaves::Disturb(UINT i, UINT j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < m_nNumRows - 2);
	assert(j > 1 && j < m_nNumCols - 2);

	float halfMag = 0.5f*magnitude;

	// Disturb the ijth vertex height and its neighbors.
	m_pvCurrSolution[i*m_nNumCols + j].y        += magnitude;
	m_pvCurrSolution[i*m_nNumCols + j + 1].y    += halfMag;
	m_pvCurrSolution[i*m_nNumCols + j - 1].y    += halfMag;
	m_pvCurrSolution[(i + 1)*m_nNumCols + j].y  += halfMag;
	m_pvCurrSolution[(i - 1)*m_nNumCols + j].y  += halfMag;
}
