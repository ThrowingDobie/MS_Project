//-------------------------------------------------------------------------------------------------
// File : asdxBuffer.h
// Desc : Buffer Utility Module.
// Copyright(c) Project Asura. All right resereved.
//-------------------------------------------------------------------------------------------------

#ifndef __ASDX_BUFFER_H__
#define __ASDX_BUFFER_H__

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>
#include <d3d11.h>


namespace asdx {

//-------------------------------------------------------------------------------------------------
//! @brief      静的インデックスバッファを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[in]      pIndices        インデックスバッファです.
//! @param[out]     ppBuffer        インデックスバッファの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
bool CreateStaticIndexBuffer( ID3D11Device* pDevice, const T* pIndices, const u32 indexCount, ID3D11Buffer** ppBuffer )
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = indexCount * sizeof(T);

    D3D11_SUBRESOURCE_DATA res;
    ZeroMemory( &res, sizeof(res) );
    res.pSysMem = pIndices;

    HRESULT hr = pDevice->CreateBuffer( &desc, &res, ppBuffer );
    if ( FAILED( hr ) )
    {
        ASDX_RELEASE( (*ppBuffer) );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      動的インデックスバッファを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[in]      indexCount      インデックスの数です.
//! @param[out]     ppBuffer        インデックスバッファの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
bool CreateDynamicIndexBuffer( ID3D11Device* pDevice, const u32 indexCount, ID3D11Buffer** ppBuffer )
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.ByteWidth      = indexCount * sizeof(T);

    HRESULT hr = pDevice->CreateBuffer( &desc, nullptr, ppBuffer );
    if ( FAILED( hr ) )
    {
        ASDX_RELEASE( (*ppBuffer) );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      静的頂点バッファを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[in]      pVertices       頂点バッファです.
//! @param[out]     ppBuffer        頂点バッファの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
bool CreateStaticVertexBuffer( ID3D11Device* pDevice, const T* pVertices, const u32 vertexCount, ID3D11Buffer** ppBuffer )
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = vertexCount * sizeof(T);

    D3D11_SUBRESOURCE_DATA res;
    ZeroMemory( &res, sizeof(res) );
    res.pSysMem = pVertices;

    HRESULT hr = pDevice->CreateBuffer( &desc, &res, ppBuffer );
    if ( FAILED( hr ) )
    {
        ASDX_RELEASE( (*ppBuffer) );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      動的頂点バッファを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[in]      vertexCount     頂点数です.
//! @param[out]     ppBuffer        頂点バッファの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
bool CreateDynamicVertexBuffer( ID3D11Device* pDevice, const u32 vertexCount, ID3D11Buffer** ppBuffer )
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.ByteWidth      = vertexCount * sizeof(T);

    HRESULT hr = pDevice->CreateBuffer( &desc, nullptr, ppBuffer );
    if ( FAILED( hr ) )
    {
        ASDX_RELEASE( (*ppBuffer) );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      定数バッファを生成します.
//!
//! @param[in]      pDevice         デバイスです.
//! @param[out]     ppBuffer        定数バッファの格納先です.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
ASDX_TEMPLATE(T)
bool CreateConstantBuffer( ID3D11Device* pDevice, ID3D11Buffer** ppBuffer )
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(T);

    HRESULT hr = pDevice->CreateBuffer( &desc, nullptr, ppBuffer );
    if ( FAILED( hr ) )
    {
        ASDX_RELEASE( (*ppBuffer) );
        return false;
    }

    return true;
}


} // namespace asdx


#endif//__ASDX_BUFFER_H__
