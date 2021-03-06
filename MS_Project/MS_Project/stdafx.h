// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#include <dxgi1_2.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dx9.h>
#include <xnamath.h>
#include <assert.h>

#include <windowsx.h>
#include <sstream>

#include <DxErr.h>

#include <queue>
#include <stack>
#include <string>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include "MathHelper.h"
#include "d3dUtil.h"
#include "GeometryGenerator.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "xnacollision.h"
#include "DDSTextureLoader.h"

#define SINGLETONE(class_name) private: class_name(void); ~class_name(void); \
public: static class_name* GetInstance() { static class_name instance; return &instance; }

#define SAFE_RELEASE(p) if(p){p->Release(); p = NULL;}
#define SAFE_DELETE(p) if(p){delete p; p = NULL;}
#define SAFE_DELETE_ARRAY(p) if(p){delete [] p; p = NULL;}
#define SAFE_ADD_REF(p) if(p){p->AddRef();}

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
		{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
				{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
				}                                                      \
		}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#include "cDeviceManager.h"
#include "GameTimer.h"
#include "Camera.h"

extern HWND g_hWnd;

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

enum CornerType
{
	E_CornerTL,
	E_CornerTR,
	E_CornerBL,
	E_CornerBR
};

struct ST_INDEX
{
	UINT i0;
	UINT i1;
	UINT i2;
	UINT i3;

	XMVECTOR v0;
	XMVECTOR v1;
	XMVECTOR v2;
	XMVECTOR v3;
};

struct ST_OCTREE_INDEX
{
	UINT i0;
	UINT i1;
	UINT i2;
	UINT i3;
	UINT i4;
	UINT i5;
	UINT i6;
	UINT i7;

	XMVECTOR v0;
	XMVECTOR v1;
	XMVECTOR v2;
	XMVECTOR v3;
	XMVECTOR v4;
	XMVECTOR v5;
	XMVECTOR v6;
	XMVECTOR v7;
};