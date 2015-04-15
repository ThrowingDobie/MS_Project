//--------------------------------------------------------------------------------------------------
// File : asdxGeometry.inl
// Desc : Geometry Module.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------------

#ifndef __ASDX_GEOMETRY_INL__
#define __ASDX_GEOMETRY_INL__


namespace asdx {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Plane structure
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane::Plane()
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane::Plane( const f32* pValues )
{
    assert( pValues != nullptr );
    n.x = pValues[ 0 ];
    n.y = pValues[ 1 ];
    n.z = pValues[ 2 ];
    d   = pValues[ 3 ];
}

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane::Plane( const f32 _a, const f32 _b, const f32 _c, const f32 _d )
: n( _a, _b, _c )
, d( _d )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      f32*型へのキャストです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane::operator f32* ()
{ return &n.x; }

//--------------------------------------------------------------------------------------------------
//      const f32*型へのキャストです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane::operator const f32* () const
{ return &n.x; }

//--------------------------------------------------------------------------------------------------
//      乗算代入演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane& Plane::operator *= ( const f32 value )
{
    n.x *= value;
    n.y *= value;
    n.z *= value;
    d   *= value;
    return (*this);
}

//--------------------------------------------------------------------------------------------------
//      除算代入演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane& Plane::operator /= ( const f32 value )
{
    assert( !asdx::IsZero( value ) );
    n.x /= value;
    n.y /= value;
    n.z /= value;
    d   /= value;
    return (*this);
}

//--------------------------------------------------------------------------------------------------
//      正符号演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::operator + () const
{ return (*this); }

//--------------------------------------------------------------------------------------------------
//      負符号演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::operator - () const
{ return Plane( -n.x, -n.y, -n.z, -d ); }

//--------------------------------------------------------------------------------------------------
//      乗算演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::operator * ( const f32 value ) const
{
    return Plane(
        n.x * value,
        n.y * value,
        n.z * value,
        d   * value );
}

//--------------------------------------------------------------------------------------------------
//      除算演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::operator / ( const f32 value ) const
{
    assert( !asdx::IsZero( value ) );
    return Plane(
        n.x / value,
        n.y / value,
        n.z / value,
        d   / value );
}

//--------------------------------------------------------------------------------------------------
//      等価比較演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Plane::operator == ( const Plane& value ) const
{
    return ( n.x == value.n.x )
        && ( n.y == value.n.y )
        && ( n.z == value.n.z )
        && ( d   == value.d );
}

//--------------------------------------------------------------------------------------------------
//      非等価比較演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Plane::operator != ( const Plane& value ) const
{
    return ( n.x != value.n.x )
        || ( n.y != value.n.y )
        || ( n.z != value.n.z )
        || ( d   != value.d );
}

//--------------------------------------------------------------------------------------------------
//      乗算演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane operator * ( f32 value, const Plane& plane )
{
    return Plane(
        value * plane.n.x,
        value * plane.n.y,
        value * plane.n.z,
        value * plane.d );
}

//--------------------------------------------------------------------------------------------------
//      正規化します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane& Plane::Normalize()
{
    f32 mag = sqrtf( n.x * n.x + n.y * n.y + n.z * n.z + d * d );
    assert( !asdx::IsZero( mag ) );
    n.x /= mag;
    n.y /= mag;
    n.z /= mag;
    d   /= mag;
    return (*this);
}

//--------------------------------------------------------------------------------------------------
//      ゼロ除算を考慮して正規化します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane& Plane::SafeNormalize( const Plane& value )
{
    f32 mag = sqrtf( n.x * n.x + n.y * n.y + n.z * n.z + d * d );
    if ( !asdx::IsZero( mag ) )
    {
        n.x /= mag;
        n.y /= mag;
        n.z /= mag;
        d   /= mag;
    }
    else
    {
        n.x = value.n.x;
        n.y = value.n.y;
        n.z = value.n.z;
        d   = value.d;
    }

    return (*this);
}

//--------------------------------------------------------------------------------------------------
//      内積を求めます.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
f32 Plane::Dot( const Vector4& value ) const
{
    return ( n.x * value.x
           + n.y * value.y
           + n.z * value.z
           + d   * value.w );
}

//--------------------------------------------------------------------------------------------------
//      内積を求めます.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
f32 Plane::DotCoord( const Vector3& value ) const
{
    return ( n.x * value.x
           + n.y * value.y
           + n.z * value.z
           + d );
}

//--------------------------------------------------------------------------------------------------
//      法線ベクトルの内積を求めます.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
f32 Plane::DotNormal( const Vector3& value ) const
{
    return ( n.x * value.x
           + n.y * value.y
           + n.z * value.z );
}

//--------------------------------------------------------------------------------------------------
//      点が平面より内側であるかチェックします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Plane::Inside( const Vector3& point ) const
{
    f32 result = this->DotCoord( point );
    return ( result >= 0.0f );
}

//--------------------------------------------------------------------------------------------------
//      球が平面より内側であるかチェックします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Plane::Inside( const BoundingSphere& sphere ) const
{
    f32 dist = this->DotCoord( sphere.center );
    return ( dist >= -sphere.radius );
}

//--------------------------------------------------------------------------------------------------
//      点と法線ベクトルから平面を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::CreateFromPointNormal( const Vector3& normal, const Vector3& point )
{
    f32 w = Vector3::Dot( point, normal );
    return Plane( normal.x, normal.y, normal.z, -w );
}

//--------------------------------------------------------------------------------------------------
//      点と法線ベクトルから平面を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
void Plane::CreateFromPointNormal( const Vector3& normal, const Vector3& point, Plane& result )
{
    f32 w = Vector3::Dot( point, normal );
    result.n.x = normal.x;
    result.n.y = normal.y;
    result.n.z = normal.z;
    result.d   = -w;
}

//--------------------------------------------------------------------------------------------------
//      3点から平面を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Plane Plane::CreateFromPoints( const Vector3& a, const Vector3& b, const Vector3& c )
{
    Vector3 v21 = b - a;
    Vector3 v31 = c - a;

    Vector3 normal = Vector3::Cross( v21, v31 );
    normal.Normalize();

    f32 dist = Vector3::Dot( normal, a );
    return Plane( normal.x, normal.y, normal.z, -dist );
}

//--------------------------------------------------------------------------------------------------
//      3点から平面を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
void Plane::CreateFromPoints( const Vector3& a, const Vector3& b, const Vector3& c, Plane& result )
{
    Vector3 v21 = b - a;
    Vector3 v31 = c - a;

    Vector3 normal = Vector3::Cross( v21, v31 );
    normal.Normalize();

    f32 dist = Vector3::Dot( normal, a );
    result.n.x = normal.x;
    result.n.y = normal.y;
    result.n.z = normal.z;
    result.d   = -dist;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// BoundingBox structures
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox()
: mini(  F32_MAX,  F32_MAX,  F32_MAX )
, maxi( -F32_MAX, -F32_MAX, -F32_MAX )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox( const asdx::Vector3& _min, const asdx::Vector3& _max )
: mini( _min )
, maxi( _max )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox::BoundingBox( const BoundingBox& value )
: mini( value.mini )
, maxi( value.maxi )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      中心座標を取得します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Vector3 BoundingBox::GetCenter() const
{ return ( maxi + mini ) * 0.5f; }

//--------------------------------------------------------------------------------------------------
//      8角の頂点を取得します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
std::array<Vector3, 8> BoundingBox::GetCorners() const
{
    std::array<Vector3, 8> result;
    result[ 0 ] = Vector3( mini.x, mini.y, mini.z );
    result[ 1 ] = Vector3( mini.x, maxi.y, mini.z );
    result[ 2 ] = Vector3( maxi.x, maxi.y, mini.z );
    result[ 3 ] = Vector3( maxi.x, mini.y, mini.z );
    result[ 4 ] = Vector3( mini.x, mini.y, maxi.z );
    result[ 5 ] = Vector3( mini.x, maxi.y, maxi.z );
    result[ 6 ] = Vector3( maxi.x, maxi.y, maxi.z );
    result[ 7 ] = Vector3( maxi.x, mini.y, maxi.z );
    return result;
}

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
void BoundingBox::Merge( const asdx::Vector3& value )
{
    maxi = asdx::Vector3::Max( maxi, value );
    mini = asdx::Vector3::Min( mini, value );
}

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingBox BoundingBox::Merge( const BoundingBox& a, const BoundingBox& b )
{
    return BoundingBox(
        asdx::Vector3::Min( a.mini, b.mini ),
        asdx::Vector3::Max( a.maxi, b.maxi ) );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// BoundingSphere structure
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere()
: center( 0.0f, 0.0f, 0.0f )
, radius( F32_MAX )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const asdx::Vector3& _center, const f32 _radius )
: center( _center )
, radius( _radius )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const BoundingBox& value )
{
    center = ( value.mini + value.maxi ) * 0.5f;
    radius = asdx::Vector3::Distance( value.mini, value.maxi ) * 0.5f;
}

//--------------------------------------------------------------------------------------------------
//      コピーコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere::BoundingSphere( const BoundingSphere& value )
: center( value.center )
, radius( value.radius )
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      マージします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
BoundingSphere BoundingSphere::Merge( const BoundingSphere& a, const BoundingSphere& b )
{
    asdx::Vector3 dif( 
        b.center.x - a.center.x,
        b.center.y - a.center.y,
        b.center.z - a.center.z );

    register f32 len = dif.Length();
    
    if ( a.radius + b.radius >= len )
    {
        if ( a.radius - b.radius >= len )
        { return a; }

        if ( b.radius - a.radius >= len )
        { return b; }
    }

    assert( len != 0.0f );
    asdx::Vector3 v = dif * ( 1.0f / len );
    register f32 fmin = asdx::Min<f32>( -a.radius, len - b.radius );
    register f32 fmax = asdx::Max<f32>(  a.radius, len + b.radius );
    register f32 fmag = ( fmax - fmin ) * 0.5f;

    return BoundingSphere(
        asdx::Vector3( a.center.x + v.x * ( fmag + fmin ),
                       a.center.y + v.y * ( fmag + fmin ),
                       a.center.z + v.z * ( fmag + fmin )
        ),
        fmag
    );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Frustum class
////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------
//      コンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum::Frustum()
{ /* DO_NOTHING */ }

//--------------------------------------------------------------------------------------------------
//      引数付きコンストラクタです.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum::Frustum( const Frustum& value )
{
    for( u32 i=0; i<Side::NumPlanes; ++i )
    { m_Planes[ i ] = value.m_Planes[ i ]; }

    for( u32 i=0; i<4; ++i )
    {
        m_NearPoint[ i ] = value.m_NearPoint[ i ];
        m_FarPoint [ i ] = value.m_FarPoint[ i ];
    }
}

//---------------------------------------------------------------------------------------------------
//      8角の頂点を取得します.
//---------------------------------------------------------------------------------------------------
ASDX_INLINE
std::array<Vector3, 8> Frustum::GetCorners() const
{
    std::array<Vector3, 8> result;
    result[ 0 ] = m_NearPoint[ 0 ];
    result[ 1 ] = m_NearPoint[ 1 ];
    result[ 2 ] = m_NearPoint[ 2 ];
    result[ 3 ] = m_NearPoint[ 3 ];
    result[ 4 ] = m_FarPoint[ 0 ];
    result[ 5 ] = m_FarPoint[ 1 ];
    result[ 6 ] = m_FarPoint[ 2 ];
    result[ 7 ] = m_FarPoint[ 3 ];
    return result;
}

//--------------------------------------------------------------------------------------------------
//      錐台内に含まれるかチェックします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Frustum::Contains( const Vector3& point ) const
{
    for( u32 i=0; i<= Side::NumPlanes; ++i )
    {
        if ( m_Planes[ i ].Inside( point ) )
        { return true; }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
//      錐台内に含まれるかチェックします.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
bool Frustum::Contains( const BoundingSphere& value ) const
{
    for( u32 i=0; i<Side::NumPlanes; ++i )
    {
        if ( m_Planes[ i ].Inside( value ) )
        { return true; }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
//      平面を取得します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
const Plane& Frustum::GetPlane( Side value ) const
{ return m_Planes[ value ]; }

//--------------------------------------------------------------------------------------------------
//      代入演算子です.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum& Frustum::operator = ( const Frustum& value )
{
    for( u32 i=0; i<Side::NumPlanes; ++i )
    { m_Planes[ i ] = value.m_Planes[ i ]; }

    for( u32 i=0; i<4; ++i )
    {
        m_NearPoint[ i ] = value.m_NearPoint[ i ];
        m_FarPoint [ i ] = value.m_FarPoint [ i ];
    }

    return (*this);
}

//--------------------------------------------------------------------------------------------------
//      透視投影カメラパラメータから錐台を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum Frustum::CreatePerspectiveFromLookAt
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraTarget,
    const Vector3&  cameraUpward,
    const f32       fieldOfView,
    const f32       aspectRatio,
    const f32       nearClip,
    const f32       farClip
)
{
    Frustum result;

    Vector3 cameraDir = Vector3::Normalize( cameraTarget - cameraPosition );
    Vector3 rightDir  = Vector3::Normalize( Vector3::Cross( cameraUpward, cameraDir ) );
    Vector3 upward    = Vector3::Normalize( Vector3::Cross( cameraDir, rightDir ) );

    Vector3 nearCenter = cameraPosition + cameraDir * nearClip;
    Vector3 farCenter  = cameraPosition + cameraDir * farClip;
    
    f32 tanFovOver2 = tanf( fieldOfView / 2.0f );
    
    f32 nearHalfHeight = nearClip * tanFovOver2;
    f32 farHalfHeight  = farClip  * tanFovOver2;
    
    f32 nearHalfWidth  = nearHalfHeight * aspectRatio;
    f32 farHalfWidth   = farHalfHeight  * aspectRatio;

    result.m_NearPoint[ 0 ] = nearCenter + nearHalfHeight * upward - nearHalfWidth * rightDir;
    result.m_NearPoint[ 1 ] = nearCenter + nearHalfHeight * upward + nearHalfWidth * rightDir;
    result.m_NearPoint[ 2 ] = nearCenter - nearHalfHeight * upward + nearHalfWidth * rightDir;
    result.m_NearPoint[ 3 ] = nearCenter - nearHalfHeight * upward - nearHalfWidth * rightDir;

    result.m_FarPoint[ 0 ] = farCenter + farHalfHeight * upward - farHalfWidth * rightDir;
    result.m_FarPoint[ 1 ] = farCenter + farHalfHeight * upward + farHalfWidth * rightDir;
    result.m_FarPoint[ 2 ] = farCenter - farHalfHeight * upward + farHalfWidth * rightDir;
    result.m_FarPoint[ 3 ] = farCenter - farHalfHeight * upward - farHalfWidth * rightDir;

    result.m_Planes[ Side::Near   ] = Plane::CreateFromPoints( result.m_NearPoint[ 2 ], result.m_NearPoint[ 1 ], result.m_NearPoint[ 0 ] );
    result.m_Planes[ Side::Far    ] = Plane::CreateFromPoints( result.m_FarPoint [ 0 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 2 ] );
    result.m_Planes[ Side::Right  ] = Plane::CreateFromPoints( result.m_FarPoint [ 2 ], result.m_FarPoint [ 1 ], result.m_NearPoint[ 1 ] );
    result.m_Planes[ Side::Top    ] = Plane::CreateFromPoints( result.m_NearPoint[ 1 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Left   ] = Plane::CreateFromPoints( result.m_NearPoint[ 3 ], result.m_NearPoint[ 0 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Bottom ] = Plane::CreateFromPoints( result.m_FarPoint [ 3 ], result.m_FarPoint [ 2 ], result.m_NearPoint[ 2 ] );

    return result;
}

//--------------------------------------------------------------------------------------------------
//      正射影カメラパラメータから錐台を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum Frustum::CreateOrthoFromLookAt
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraTarget,
    const Vector3&  cameraUpward,
    const f32       width,
    const f32       height,
    const f32       nearClip,
    const f32       farClip
)
{
    Frustum result;

    Vector3 cameraDir = Vector3::Normalize( cameraTarget - cameraPosition );
    Vector3 rightDir  = Vector3::Normalize( Vector3::Cross( cameraUpward, cameraDir ) );
    Vector3 upward    = Vector3::Normalize( Vector3::Cross( cameraDir, rightDir ) );

    Vector3 nearCenter = cameraPosition + cameraDir * nearClip;
    Vector3 farCenter  = cameraPosition + cameraDir * farClip;

    f32 halfWidth  = width * 0.5f;
    f32 halfHeight = height * 0.5f;

    result.m_NearPoint[ 0 ] = nearCenter + halfHeight * upward - halfWidth * rightDir;
    result.m_NearPoint[ 1 ] = nearCenter + halfHeight * upward + halfWidth * rightDir;
    result.m_NearPoint[ 2 ] = nearCenter - halfHeight * upward + halfWidth * rightDir;
    result.m_NearPoint[ 3 ] = nearCenter - halfHeight * upward - halfWidth * rightDir;

    result.m_FarPoint[ 0 ] = farCenter + halfHeight * upward - halfWidth * rightDir;
    result.m_FarPoint[ 1 ] = farCenter + halfHeight * upward + halfWidth * rightDir;
    result.m_FarPoint[ 2 ] = farCenter - halfHeight * upward + halfWidth * rightDir;
    result.m_FarPoint[ 3 ] = farCenter - halfHeight * upward - halfWidth * rightDir;

    result.m_Planes[ Side::Near   ] = Plane::CreateFromPoints( result.m_NearPoint[ 2 ], result.m_NearPoint[ 1 ], result.m_NearPoint[ 0 ] );
    result.m_Planes[ Side::Far    ] = Plane::CreateFromPoints( result.m_FarPoint [ 0 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 2 ] );
    result.m_Planes[ Side::Right  ] = Plane::CreateFromPoints( result.m_FarPoint [ 2 ], result.m_FarPoint [ 1 ], result.m_NearPoint[ 1 ] );
    result.m_Planes[ Side::Top    ] = Plane::CreateFromPoints( result.m_NearPoint[ 1 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Left   ] = Plane::CreateFromPoints( result.m_NearPoint[ 3 ], result.m_NearPoint[ 0 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Bottom ] = Plane::CreateFromPoints( result.m_FarPoint [ 3 ], result.m_FarPoint [ 2 ], result.m_NearPoint[ 2 ] );

    return result;
}

//--------------------------------------------------------------------------------------------------
//      透視投影カメラパラメータから錐台を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum Frustum::CreatePerspectiveFromLookTo
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraDirection,
    const Vector3&  cameraUpward,
    const f32       fieldOfView,
    const f32       aspectRatio,
    const f32       nearClip,
    const f32       farClip
)
{
    Frustum result;

    Vector3 cameraDir = Vector3::Normalize( cameraDirection );
    Vector3 rightDir  = Vector3::Normalize( Vector3::Cross( cameraUpward, cameraDir ) );
    Vector3 upward    = Vector3::Normalize( Vector3::Cross( cameraDir, rightDir ) );

    Vector3 nearCenter = cameraPosition + cameraDir * nearClip;
    Vector3 farCenter  = cameraPosition + cameraDir * farClip;
    
    f32 tanFovOver2 = tanf( fieldOfView / 2.0f );
    
    f32 nearHalfHeight = nearClip * tanFovOver2;
    f32 farHalfHeight  = farClip  * tanFovOver2;
    
    f32 nearHalfWidth  = nearHalfHeight * aspectRatio;
    f32 farHalfWidth   = farHalfHeight  * aspectRatio;

    result.m_NearPoint[ 0 ] = nearCenter + nearHalfHeight * upward - nearHalfWidth * rightDir;
    result.m_NearPoint[ 1 ] = nearCenter + nearHalfHeight * upward + nearHalfWidth * rightDir;
    result.m_NearPoint[ 2 ] = nearCenter - nearHalfHeight * upward + nearHalfWidth * rightDir;
    result.m_NearPoint[ 3 ] = nearCenter - nearHalfHeight * upward - nearHalfWidth * rightDir;

    result.m_FarPoint[ 0 ] = farCenter + farHalfHeight * upward - farHalfWidth * rightDir;
    result.m_FarPoint[ 1 ] = farCenter + farHalfHeight * upward + farHalfWidth * rightDir;
    result.m_FarPoint[ 2 ] = farCenter - farHalfHeight * upward + farHalfWidth * rightDir;
    result.m_FarPoint[ 3 ] = farCenter - farHalfHeight * upward - farHalfWidth * rightDir;

    result.m_Planes[ Side::Near   ] = Plane::CreateFromPoints( result.m_NearPoint[ 2 ], result.m_NearPoint[ 1 ], result.m_NearPoint[ 0 ] );
    result.m_Planes[ Side::Far    ] = Plane::CreateFromPoints( result.m_FarPoint [ 0 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 2 ] );
    result.m_Planes[ Side::Right  ] = Plane::CreateFromPoints( result.m_FarPoint [ 2 ], result.m_FarPoint [ 1 ], result.m_NearPoint[ 1 ] );
    result.m_Planes[ Side::Top    ] = Plane::CreateFromPoints( result.m_NearPoint[ 1 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Left   ] = Plane::CreateFromPoints( result.m_NearPoint[ 3 ], result.m_NearPoint[ 0 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Bottom ] = Plane::CreateFromPoints( result.m_FarPoint [ 3 ], result.m_FarPoint [ 2 ], result.m_NearPoint[ 2 ] );

    return result;
}

//--------------------------------------------------------------------------------------------------
//      正射影カメラパラメータから錐台を生成します.
//--------------------------------------------------------------------------------------------------
ASDX_INLINE
Frustum Frustum::CreateOrthoFromLookTo
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraDirection,
    const Vector3&  cameraUpward,
    const f32       width,
    const f32       height,
    const f32       nearClip,
    const f32       farClip
)
{
    Frustum result;

    Vector3 cameraDir = Vector3::Normalize( cameraDirection );
    Vector3 rightDir  = Vector3::Normalize( Vector3::Cross( cameraUpward, cameraDir ) );
    Vector3 upward    = Vector3::Normalize( Vector3::Cross( cameraDir, rightDir ) );

    Vector3 nearCenter = cameraPosition + cameraDir * nearClip;
    Vector3 farCenter  = cameraPosition + cameraDir * farClip;

    f32 halfWidth  = width  * 0.5f;
    f32 halfHeight = height * 0.5f;

    result.m_NearPoint[ 0 ] = nearCenter + halfHeight * upward - halfWidth * rightDir;
    result.m_NearPoint[ 1 ] = nearCenter + halfHeight * upward + halfWidth * rightDir;
    result.m_NearPoint[ 2 ] = nearCenter - halfHeight * upward + halfWidth * rightDir;
    result.m_NearPoint[ 3 ] = nearCenter - halfHeight * upward - halfWidth * rightDir;

    result.m_FarPoint[ 0 ] = farCenter + halfHeight * upward - halfWidth * rightDir;
    result.m_FarPoint[ 1 ] = farCenter + halfHeight * upward + halfWidth * rightDir;
    result.m_FarPoint[ 2 ] = farCenter - halfHeight * upward + halfWidth * rightDir;
    result.m_FarPoint[ 3 ] = farCenter - halfHeight * upward - halfWidth * rightDir;

    result.m_Planes[ Side::Near   ] = Plane::CreateFromPoints( result.m_NearPoint[ 2 ], result.m_NearPoint[ 1 ], result.m_NearPoint[ 0 ] );
    result.m_Planes[ Side::Far    ] = Plane::CreateFromPoints( result.m_FarPoint [ 0 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 2 ] );
    result.m_Planes[ Side::Right  ] = Plane::CreateFromPoints( result.m_FarPoint [ 2 ], result.m_FarPoint [ 1 ], result.m_NearPoint[ 1 ] );
    result.m_Planes[ Side::Top    ] = Plane::CreateFromPoints( result.m_NearPoint[ 1 ], result.m_FarPoint [ 1 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Left   ] = Plane::CreateFromPoints( result.m_NearPoint[ 3 ], result.m_NearPoint[ 0 ], result.m_FarPoint [ 0 ] );
    result.m_Planes[ Side::Bottom ] = Plane::CreateFromPoints( result.m_FarPoint [ 3 ], result.m_FarPoint [ 2 ], result.m_NearPoint[ 2 ] );

    return result;
}

} // namespace asdx

#endif//__VIEWER_GEOMETRY_INL__
