#include "stdafx.h"
#include "cSkull.h"


cSkull::cSkull()
    : m_pRoomVertexBuffer(NULL)
    , m_pSkullVertexBuffer(NULL)
    , m_pSkullIndexBuffer(NULL)
    , m_pFloorDiffuseMapSRV(NULL)
    , m_pWallDiffuseMapSRV(NULL)
    , m_pMirrorDiffuseMapSRV(NULL)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&m_matRoomWorld, I);
    XMStoreFloat4x4(&m_matSkullWorld, I);
    m_vSkullTranslation = { 0.f, 1.0f, -5.0f };
}

cSkull::~cSkull()
{
    SAFE_RELESE(m_pRoomVertexBuffer);
    SAFE_RELESE(m_pSkullVertexBuffer);
    SAFE_RELESE(m_pSkullIndexBuffer);
    SAFE_RELESE(m_pFloorDiffuseMapSRV);
    SAFE_RELESE(m_pWallDiffuseMapSRV);
    SAFE_RELESE(m_pMirrorDiffuseMapSRV);
}

void cSkull::Setup()
{
    m_RenderOptions = Textures;

    m_DirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    m_DirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    m_DirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    m_mtRoom.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_mtRoom.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_mtRoom.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    m_mtSkull.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_mtSkull.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_mtSkull.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    // Reflected material is transparent so it blends into mirror.
    m_mtMirror.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_mtMirror.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    m_mtMirror.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    m_mtShadow.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_mtShadow.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
    m_mtShadow.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

void cSkull::Init()
{
    HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
        L"Textures/checkboard.dds", 0, 0, &m_pFloorDiffuseMapSRV, 0));

    HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
        L"Textures/brick01.dds", 0, 0, &m_pWallDiffuseMapSRV, 0));

    HR(D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice->m_pDevice,
        L"Textures/ice.dds", 0, 0, &m_pMirrorDiffuseMapSRV, 0));

    BuildRoomGeometryBuffers();
    BuildSkullGeometryBuffers();
}

void cSkull::Update(float fDelta)
{
    if (GetAsyncKeyState('1') & 0x8000)
        m_RenderOptions = RenderOptions::Lighting;

    if (GetAsyncKeyState('2') & 0x8000)
        m_RenderOptions = RenderOptions::Textures;

    if (GetAsyncKeyState('3') & 0x8000)
        m_RenderOptions = RenderOptions::TexturesAndFog;

    if (GetAsyncKeyState('A') & 0x8000)
        m_vSkullTranslation.x -= 1.0f*fDelta;

    if (GetAsyncKeyState('D') & 0x8000)
        m_vSkullTranslation.x += 1.0f*fDelta;

    if (GetAsyncKeyState('W') & 0x8000)
        m_vSkullTranslation.y += 1.0f*fDelta;

    if (GetAsyncKeyState('S') & 0x8000)
        m_vSkullTranslation.y -= 1.0f*fDelta;

    // Don't let user move below ground plane.
    m_vSkullTranslation.y = MathHelper::Max(m_vSkullTranslation.y, 0.0f);

    XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
    XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
    XMMATRIX skullOffset = XMMatrixTranslation(m_vSkullTranslation.x, m_vSkullTranslation.y, m_vSkullTranslation.z);
    XMStoreFloat4x4(&m_matSkullWorld, skullRotate*skullScale*skullOffset);
}

void cSkull::Render()
{
    g_pD3DDevice->m_pDevCon->IASetInputLayout(InputLayouts::Basic32);
    g_pD3DDevice->m_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4(&g_pCamera->m_matView);
    XMMATRIX proj = XMLoadFloat4x4(&g_pCamera->m_matProj);
    XMMATRIX viewProj = view*proj;

    // Set per frame constants.
    Effects::BasicFX->SetDirLights(m_DirLights);
    Effects::BasicFX->SetEyePosW(g_pCamera->m_vEyePosW);
    Effects::BasicFX->SetFogColor(Colors::Black);
    Effects::BasicFX->SetFogStart(2.0f);
    Effects::BasicFX->SetFogRange(40.0f);

    // Skull doesn't have texture coordinates, so we can't texture it.
    ID3DX11EffectTechnique* activeTech = nullptr;
    ID3DX11EffectTechnique* activeSkullTech = nullptr;

    switch (m_RenderOptions)
    {
    case RenderOptions::Lighting:
        activeTech = Effects::BasicFX->Light3Tech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        activeTech = Effects::BasicFX->Light3TexTech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::TexturesAndFog:
        activeTech = Effects::BasicFX->Light3TexFogTech;
        activeSkullTech = Effects::BasicFX->Light3FogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;

    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pRoomVertexBuffer, &stride, &offset);

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4(&m_matRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(m_mtRoom);

        // Floor
        Effects::BasicFX->SetDiffuseMap(m_pFloorDiffuseMapSRV);
        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->Draw(6, 0);

        // Wall
        Effects::BasicFX->SetDiffuseMap(m_pWallDiffuseMapSRV);
        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->Draw(18, 6);
    }

    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pSkullVertexBuffer, &stride, &offset);
        g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMMATRIX world = XMLoadFloat4x4(&m_matSkullWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(m_mtSkull);

        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->DrawIndexed(m_nSkullIndexCount, 0, 0);
    }

    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pRoomVertexBuffer, &stride, &offset);

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4(&m_matRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

        // Do not write to render target.
        g_pD3DDevice->m_pDevCon->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);

        // Render visible mirror pixels to stencil buffer.
        // Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.
        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);

        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->Draw(6, 24);

        // Restore states.
        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(0, 0);
        g_pD3DDevice->m_pDevCon->OMSetBlendState(0, blendFactor, 0xffffffff);
    }

    //
    // Draw the skull reflection.
    //
    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pSkullVertexBuffer, &stride, &offset);
        g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
        XMMATRIX R = XMMatrixReflect(mirrorPlane);
        XMMATRIX world = XMLoadFloat4x4(&m_matSkullWorld) * R;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(m_mtSkull);

        // Cache the old light directions, and reflect the light directions.
        XMFLOAT3 oldLightDirections[3];
        for (int i = 0; i < 3; ++i)
        {
            oldLightDirections[i] = m_DirLights[i].Direction;

            XMVECTOR lightDir = XMLoadFloat3(&m_DirLights[i].Direction);
            XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
            XMStoreFloat3(&m_DirLights[i].Direction, reflectedLightDir);
        }

        Effects::BasicFX->SetDirLights(m_DirLights);

        // Cull clockwise triangles for reflection.
        g_pD3DDevice->m_pDevCon->RSSetState(RenderStates::CullClockwiseRS);

        // Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(RenderStates::DrawReflectionDSS, 1);
        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->DrawIndexed(m_nSkullIndexCount, 0, 0);

        // Restore default states.
        g_pD3DDevice->m_pDevCon->RSSetState(0);
        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(0, 0);

        // Restore light directions.
        for (int i = 0; i < 3; ++i)
        {
            m_DirLights[i].Direction = oldLightDirections[i];
        }

        Effects::BasicFX->SetDirLights(m_DirLights);
    }

    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pRoomVertexBuffer, &stride, &offset);

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4(&m_matRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(m_mtMirror);
        Effects::BasicFX->SetDiffuseMap(m_pMirrorDiffuseMapSRV);

        // Mirror
        g_pD3DDevice->m_pDevCon->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->Draw(6, 24);
    }

    //
    // Draw the skull shadow.
    //
    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        g_pD3DDevice->m_pDevCon->IASetVertexBuffers(0, 1, &m_pSkullVertexBuffer, &stride, &offset);
        g_pD3DDevice->m_pDevCon->IASetIndexBuffer(m_pSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
        XMVECTOR toMainLight = -XMLoadFloat3(&m_DirLights[0].Direction);
        XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
        XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4(&m_matSkullWorld)*S*shadowOffsetY;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(m_mtShadow);

        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS, 0);
        pass->Apply(0, g_pD3DDevice->m_pDevCon);
        g_pD3DDevice->m_pDevCon->DrawIndexed(m_nSkullIndexCount, 0, 0);

        // Restore default states.
        g_pD3DDevice->m_pDevCon->OMSetBlendState(0, blendFactor, 0xffffffff);
        g_pD3DDevice->m_pDevCon->OMSetDepthStencilState(0, 0);
    }
}

void cSkull::BuildRoomGeometryBuffers()
{
    Vertex::Basic32 v[30];

    // Floor: Observe we tile texture coordinates.
    v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

    v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
    v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

    // Wall: Observe we tile texture coordinates, and that we
    // leave a gap in the middle for the mirror.
    v[6] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[7] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[8] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

    v[9] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
    v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

    v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

    v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
    v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

    v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

    v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
    v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

    // Mirror
    v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

    v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pRoomVertexBuffer));
}

void cSkull::BuildSkullGeometryBuffers()
{
    std::ifstream fin("Models/skull.txt");

    if (!fin)
    {
        MessageBox(0, L"Models/skull.txt not found.", 0, 0);
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    std::vector<Vertex::Basic32> vertices(vcount);
    for (UINT i = 0; i < vcount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
    }

    fin >> ignore;
    fin >> ignore;
    fin >> ignore;

    m_nSkullIndexCount = 3 * tcount;
    std::vector<UINT> indices(m_nSkullIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(g_pD3DDevice->m_pDevice->CreateBuffer(&vbd, &vinitData, &m_pSkullVertexBuffer));

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * m_nSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(g_pD3DDevice->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_pSkullIndexBuffer));
}