#pragma once
class cSkull
{
public:
    cSkull();
    ~cSkull();

    void Setup();
    void Init();
    void Update(float fDelta);
    void Render();

private:
    ID3D11Buffer* m_pRoomVertexBuffer;
    ID3D11Buffer* m_pSkullVertexBuffer;
    ID3D11Buffer* m_pSkullIndexBuffer;

    ID3D11ShaderResourceView* m_pFloorDiffuseMapSRV;
    ID3D11ShaderResourceView* m_pWallDiffuseMapSRV;
    ID3D11ShaderResourceView* m_pMirrorDiffuseMapSRV;

    DirectionalLight m_DirLights[3];
    Material m_mtRoom;
    Material m_mtSkull;
    Material m_mtMirror;
    Material m_mtShadow;

    XMFLOAT4X4 m_matRoomWorld;
    XMFLOAT4X4 m_matSkullWorld;

    UINT m_nSkullIndexCount;
    XMFLOAT3 m_vSkullTranslation;

    RenderOptions m_RenderOptions;

private:
    void BuildRoomGeometryBuffers();
    void BuildSkullGeometryBuffers();
};

