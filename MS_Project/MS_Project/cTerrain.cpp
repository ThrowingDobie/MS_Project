#include "stdafx.h"
#include "cTerrain.h"

cTerrain::cTerrain()
	: m_pQuadPatchVertexBuffer(NULL)
	, m_pQuadPatchIndexBuffer(NULL)
	, m_pLayerMapArraySRV(NULL)
	, m_pBlendMapSRV(NULL)
	, m_pHeightMapSRV(NULL)
{
	XMMATRIX I = XMMatrixIdentity();
	I = XMMatrixTranslation(0, 0, 0);
	XMStoreFloat4x4(&m_matWorld, I);
	
	I = XMMatrixIdentity();
	I = XMMatrixScaling(1.f, 1.f, 1.f);
	XMStoreFloat4x4(&m_matScale, I);

	I = XMMatrixIdentity();
	I = XMMatrixRotationY(0);
	XMStoreFloat4x4(&m_matRot, I);

    I = XMMatrixIdentity();
    I = XMMatrixTranslation(0, 0, 0);
    XMStoreFloat4x4(&m_matTrs, I);

	m_mtTerrain.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtTerrain.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mtTerrain.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	m_mtTerrain.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

cTerrain::~cTerrain()
{
	SAFE_RELEASE(m_pQuadPatchVertexBuffer);
	SAFE_RELEASE(m_pQuadPatchIndexBuffer);
	SAFE_RELEASE(m_pLayerMapArraySRV);
	SAFE_RELEASE(m_pBlendMapSRV);
	SAFE_RELEASE(m_pHeightMapSRV);
}

void cTerrain::Setup()
{
    DXGI_FORMAT format = DXGI_FORMAT_FROM_FILE;
    //format = DXGI_FORMAT_R8G8B8A8_UINT;
    UINT filter = D3DX11_FILTER_NONE;
    UINT mipFilter = D3DX11_FILTER_LINEAR;

    D3DX11_IMAGE_LOAD_INFO loadInfo;

    //loadInfo.Width = 1024;
    //loadInfo.Height = 1024;
    loadInfo.Width = D3DX11_FROM_FILE;
    loadInfo.Height = D3DX11_FROM_FILE;
    loadInfo.Depth = D3DX11_FROM_FILE;
    loadInfo.FirstMipLevel = 0;
    loadInfo.MipLevels = D3DX11_FROM_FILE;
    loadInfo.Usage = D3D11_USAGE_STAGING;
    loadInfo.BindFlags = 0;
    loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    loadInfo.MiscFlags = 0;
    loadInfo.Format = format;
    loadInfo.Filter = filter;
    loadInfo.MipFilter = mipFilter;
    loadInfo.pSrcInfo = 0;

    ID3D11Resource* Test = nullptr;
	ID3D11ShaderResourceView* TestSRV = nullptr;

	DirectX::CreateDDSTextureFromFile(g_pD3DDevice->m_pDevice,
		L"./Image/asd.dds", &Test, &TestSRV, 0, 0);

    // png->dds
    D3DX11CreateTextureFromFile(g_pD3DDevice->m_pDevice, L"./Image/test_png.png", &loadInfo, 0
        , &Test, 0);
    D3DX11SaveTextureToFile(g_pD3DDevice->m_pDevCon, Test, D3DX11_IFF_DDS, L"./Image/asd.dds");

    //// dds->png
    //D3DX11CreateTextureFromFile(g_pD3DDevice->m_pDevice, L"./Textures/blend.dds", &loadInfo, 0, &Test, 0);
    //D3DX11SaveTextureToFile(g_pD3DDevice->m_pDevCon, Test, D3DX11_IFF_PNG, L"./Textures/asdpng.png");


}

void cTerrain::Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo)
{
	m_Info = initInfo;

	// Divide heightmap into patches such that each patch has CellsPerPatch.
	m_nPatchVertRows = ((m_Info.HeightmapHeight - 1) / CellsPerPatch) + 1;
	m_nPatchVertCols = ((m_Info.HeightmapWidth - 1) / CellsPerPatch) + 1;

	m_nPatchVertices = m_nPatchVertRows*m_nPatchVertCols;
	m_nPatchQuadFaces = (m_nPatchVertRows - 1)*(m_nPatchVertCols - 1);

	LoadHeightmap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(device);
	BuildQuadPatchIB(device);
	BuildHeightmapSRV(device);

	std::vector<std::wstring> layerFilenames;
	layerFilenames.push_back(m_Info.LayerMapFilename0);
	layerFilenames.push_back(m_Info.LayerMapFilename1);
	layerFilenames.push_back(m_Info.LayerMapFilename2);
	layerFilenames.push_back(m_Info.LayerMapFilename3);
	layerFilenames.push_back(m_Info.LayerMapFilename4);
	m_pLayerMapArraySRV = d3dHelper::CreateTexture2DArraySRV(device, dc, layerFilenames);

	HR(D3DX11CreateShaderResourceViewFromFile(device,
		m_Info.BlendMapFilename.c_str(), 0, 0, &m_pBlendMapSRV, 0));

	//DDS_PIXELFORMAT	pixelformat;
	//pixelformat.dwSize = 32;
	//pixelformat.dwFlags = DDPF_ALPHA | DDPF_RGB;
	//pixelformat.dwFourCC = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	//pixelformat.dwRGBBitCount = 32;
	//pixelformat.dwRBitMask = 0x00ff0000;
	//pixelformat.dwGBitMask = 0x0000ff00;
	//pixelformat.dwBBitMask = 0x000000ff;
	//pixelformat.dwABitMask = 0xFF000000;

	//DDS_HEADER header;
	//header.dwSize = 124;
	//header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_DEPTH;
	//header.dwHeight = 1024;
	//header.dwWidth = 1024;
	//header.dwPitchOrLinearSize = ((1024 + 1) >> 1) * 4;
	//header.dwDepth = 0;
	//header.dwMipMapCount = 10;
	//header.ddspf = pixelformat;
	//header.dwCaps = DDSCAPS_TEXTURE;

	//DDS_HEADER_DXT10 header10;
	//header10.dxgiFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	//header10.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
	//header10.miscFlag = 0;
	//header10.arraySize = sizeof(ID3D11ShaderResourceView) * 6;
	//header10.miscFlags2 = DDS_ALPHA_MODE_UNKNOWN;

	//ID3D11ShaderResourceView* iArrayElement = m_pBlendMapSRV;
	//for (UINT iArrayElement = 0; iArrayElement < header10.arraySize; iArrayElement++)
	//{
	//	for (UINT iMipLevel = 0; iMipLevel < header.dwMipMapCount; iMipLevel++)
	//	{
	//	}
	//}

}

void cTerrain::Update(float fDelta)
{
	BuildQuadPatchVB(g_pD3DDevice->m_pDevice);
	BuildQuadPatchIB(g_pD3DDevice->m_pDevice);
	BuildHeightmapSRV(g_pD3DDevice->m_pDevice);
}

void cTerrain::Render(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3])
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	dc->IASetInputLayout(InputLayouts::Terrain);

	UINT stride = sizeof(Vertex::Terrain);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &m_pQuadPatchVertexBuffer, &stride, &offset);
	dc->IASetIndexBuffer(m_pQuadPatchIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX viewProj = cam.ViewProj();
	XMMATRIX world = XMLoadFloat4x4(&m_matWorld);
    XMMATRIX trs = XMLoadFloat4x4(&m_matTrs);
    XMMATRIX rot = XMLoadFloat4x4(&m_matRot);
    XMMATRIX scale = XMLoadFloat4x4(&m_matScale);

    XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
    XMMATRIX worldViewProj = world*viewProj;

    world = rot* trs* scale;

	XMFLOAT4 worldPlanes[6];
	ExtractFrustumPlanes(worldPlanes, viewProj);

	Effects::TerrainFX->SetWorld(world);

	//XMMATRIX scale = XMLoadFloat4x4(&m_matScale);
	//Effects::TerrainFX->SetScale(scale);

	//XMMATRIX rot = XMLoadFloat4x4(&m_matRot);
	//Effects::TerrainFX->SetRot(rot);

	// Set per frame constants.
	Effects::TerrainFX->SetViewProj(viewProj);
	Effects::TerrainFX->SetEyePosW(cam.GetPosition());
	Effects::TerrainFX->SetDirLights(lights);
	Effects::TerrainFX->SetFogColor(Colors::Silver);
	Effects::TerrainFX->SetFogStart(15.0f);
	Effects::TerrainFX->SetFogRange(175.0f);
	Effects::TerrainFX->SetMinDist(20.0f);
	Effects::TerrainFX->SetMaxDist(500.0f);
	Effects::TerrainFX->SetMinTess(0.0f);
	Effects::TerrainFX->SetMaxTess(6.0f);
	Effects::TerrainFX->SetTexelCellSpaceU(1.0f / m_Info.HeightmapWidth);
	Effects::TerrainFX->SetTexelCellSpaceV(1.0f / m_Info.HeightmapHeight);
	Effects::TerrainFX->SetWorldCellSpace(m_Info.CellSpacing);
	Effects::TerrainFX->SetWorldFrustumPlanes(worldPlanes);

	Effects::TerrainFX->SetLayerMapArray(m_pLayerMapArraySRV);
	Effects::TerrainFX->SetBlendMap(m_pBlendMapSRV);
	Effects::TerrainFX->SetHeightMap(m_pHeightMapSRV);

	Effects::TerrainFX->SetMaterial(m_mtTerrain);

	ID3DX11EffectTechnique* tech = Effects::TerrainFX->Light1Tech;
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0, dc);

		dc->DrawIndexed(m_nPatchQuadFaces * 4, 0, 0);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	dc->HSSetShader(0, 0, 0);
	dc->DSSetShader(0, 0, 0);
}

void cTerrain::LoadHeightmap()
{
	// A height for each vertex
	std::vector<unsigned char> in(m_Info.HeightmapWidth * m_Info.HeightmapHeight);

	// Open the file.
	std::ifstream inFile;
	inFile.open(m_Info.HeightMapFilename.c_str(), std::ios_base::binary);

	if (inFile)
	{
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it.
	m_vecHeightmap.resize(m_Info.HeightmapHeight * m_Info.HeightmapWidth, 0);
	for (UINT i = 0; i < m_Info.HeightmapHeight * m_Info.HeightmapWidth; ++i)
	{
		m_vecHeightmap[i] = (in[i] / 255.0f)*m_Info.HeightScale;
	}
}



bool cTerrain::InBounds(int i, int j)
{
	// True if ij are valid indices; false otherwise.
	return
		i >= 0 && i < (int)m_Info.HeightmapHeight &&
		j >= 0 && j < (int)m_Info.HeightmapWidth;
}

float cTerrain::Average(int i, int j)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += m_vecHeightmap[m*m_Info.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void cTerrain::Smooth()
{
	std::vector<float> dest(m_vecHeightmap.size());

	for (UINT i = 0; i < m_Info.HeightmapHeight; ++i)
	{
		for (UINT j = 0; j < m_Info.HeightmapWidth; ++j)
		{
			dest[i*m_Info.HeightmapWidth + j] = Average(i, j);
		}
	}
	// Replace the old heightmap with the filtered one.
	m_vecHeightmap = dest;
}

void cTerrain::BuildHeightmapSRV(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_Info.HeightmapWidth;
	texDesc.Height = m_Info.HeightmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF is defined in xnamath.h, for storing 16-bit float.
	std::vector<HALF> hmap(m_vecHeightmap.size());
	std::transform(m_vecHeightmap.begin(), m_vecHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
	data.SysMemPitch = m_Info.HeightmapWidth*sizeof(HALF);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &m_pHeightMapSRV));

	// SRV saves reference.
	ReleaseCOM(hmapTex);
}

void cTerrain::BuildQuadPatchVB(ID3D11Device* device)
{
	std::vector<Vertex::Terrain> patchVertices(m_nPatchVertRows*m_nPatchVertCols);

	float halfWidth = 0.5f*GetHorizon();
	float halfDepth = 0.5f*GetVertical();

	//float halfWidth = GetHorizon();
	//float halfDepth = GetVertical();

	float patchWidth = GetHorizon() / (m_nPatchVertCols - 1);
	float patchDepth = GetVertical() / (m_nPatchVertRows - 1);
	float du = 1.0f / (m_nPatchVertCols - 1);
	float dv = 1.0f / (m_nPatchVertRows - 1);

	for (UINT i = 0; i < m_nPatchVertRows; ++i)
	{
		float z = /*halfDepth*/GetVertical() - (i*patchDepth);
		for (UINT j = 0; j < m_nPatchVertCols; ++j)
		{
			float x = /*-halfWidth +*/ j*patchWidth;

			patchVertices[i*m_nPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);

			// Stretch texture over grid.
			patchVertices[i*m_nPatchVertCols + j].Tex.x = j*du;
			patchVertices[i*m_nPatchVertCols + j].Tex.y = i*dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for (UINT i = 0; i < m_nPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < m_nPatchVertCols - 1; ++j)
		{
			UINT patchID = i*(m_nPatchVertCols - 1) + j;
			patchVertices[i*m_nPatchVertCols + j].BoundsY = m_vecPatchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Terrain) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, &m_pQuadPatchVertexBuffer));
}

void cTerrain::BuildQuadPatchIB(ID3D11Device* device)
{
	std::vector<USHORT> indices(m_nPatchQuadFaces * 4); // 4 indices per quad face

	// Iterate over each quad and compute indices.
	int k = 0;
	for (UINT i = 0; i < m_nPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < m_nPatchVertCols - 1; ++j)
		{
			// Top row of 2x2 quad patch
			indices[k] = i*m_nPatchVertCols + j;
			indices[k + 1] = i*m_nPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1)*m_nPatchVertCols + j;
			indices[k + 3] = (i + 1)*m_nPatchVertCols + j + 1;

			k += 4; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, &m_pQuadPatchIndexBuffer));
}

void cTerrain::CalcAllPatchBoundsY()
{
	m_vecPatchBoundsY.resize(m_nPatchQuadFaces);

	// For each patch
	for (UINT i = 0; i < m_nPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < m_nPatchVertCols - 1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void cTerrain::CalcPatchBoundsY(UINT i, UINT j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j*CellsPerPatch;
	UINT x1 = (j + 1)*CellsPerPatch;

	UINT y0 = i*CellsPerPatch;
	UINT y1 = (i + 1)*CellsPerPatch;

	float minY = +MathHelper::Infinity;
	float maxY = -MathHelper::Infinity;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y*m_Info.HeightmapWidth + x;
			minY = MathHelper::Min(minY, m_vecHeightmap[k]);
			maxY = MathHelper::Max(maxY, m_vecHeightmap[k]);
		}
	}

	UINT patchID = i*(m_nPatchVertCols - 1) + j;
	m_vecPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

float cTerrain::GetHorizon()const
{
	// Total terrain width.
	return (m_Info.HeightmapWidth - 1)*m_Info.CellSpacing;
}

float cTerrain::GetVertical()const
{
	// Total terrain depth.
	return (m_Info.HeightmapHeight - 1)*m_Info.CellSpacing;
}

float cTerrain::GetHeight(float x, float z)const
{
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f*GetHorizon()) / m_Info.CellSpacing;
	float d = (z - 0.5f*GetVertical()) / -m_Info.CellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = m_vecHeightmap[row*m_Info.HeightmapWidth + col];
	float B = m_vecHeightmap[row*m_Info.HeightmapWidth + col + 1];
	float C = m_vecHeightmap[(row + 1)*m_Info.HeightmapWidth + col];
	float D = m_vecHeightmap[(row + 1)*m_Info.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s*uy + t*vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s)*uy + (1.0f - t)*vy;
	}
}

XMMATRIX cTerrain::GetWorld()const
{
	return XMLoadFloat4x4(&m_matWorld);
}

void cTerrain::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&m_matWorld, M);
}

void cTerrain::ChangeHeightData(std::vector<Vertex::ST_P_VERTEX> vecVertex)
{
	m_vecHeightmap.clear();
	m_vecHeightmap.resize(vecVertex.size());

	for (UINT i = 0; i < vecVertex.size(); i++)
	{
		m_vecHeightmap[i] = vecVertex[i].Pos.y;
	}
}
