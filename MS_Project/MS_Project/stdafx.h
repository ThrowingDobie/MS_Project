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


struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

struct DDS_HEADER{
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;

};

struct DDS_HEADER_DXT10{
	DXGI_FORMAT              dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	UINT                     miscFlag;
	UINT                     arraySize;
	UINT                     miscFlags2;
};

//// DDS_HEADER Flag
//#define DDSD_CAPS			 0x1
//#define DDSD_HEIGHT			 0x2
//#define DDSD_WIDTH			 0x4
//#define DDSD_PITCH			 0x8
//#define DDSD_PIXELFORMAT	 0x1000
//#define DDSD_MIPMAPCOUNT	 0x20000
//#define DDSD_LINEARSIZE		 0x80000
//#define DDSD_DEPTH			 0x800000
//
//// DDS_HEADER Caps
//#define DDSCAPS_COMPLEX			 0x8
//#define DDSCAPS_MIPMAP			 0x400000
//#define DDSCAPS_TEXTURE			 0x1000
//
//// DDS_HEADER Caps2
//#define DDSCAPS2_CUBEMAP			 0x200
//#define DDSCAPS2_CUBEMAP_POSITIVEX	 0x400
//#define DDSCAPS2_CUBEMAP_NEGATIVEX	 0x800
//#define DDSCAPS2_CUBEMAP_POSITIVEY	 0x1000
//#define DDSCAPS2_CUBEMAP_NEGATIVEY	 0x2000
//#define DDSCAPS2_CUBEMAP_POSITIVEZ	 0x4000
//#define DDSCAPS2_CUBEMAP_NEGATIVEZ	 0x8000
//#define DDSCAPS2_VOLUME				 0x200000
//
//// DDS_PIXELFORMAT Flag
//#define DDPF_ALPHAPIXELS	 0x1
//#define DDPF_ALPHA			 0x2
//#define DDPF_FOURCC			 0x4
//#define DDPF_RGB			 0x40
//#define DDPF_YUV			 0x200
//#define DDPF_LUMINANCE		 0x20000
//
//
//
//// DDS_HEADER_DXT10 Flag1
//#define DDS_RESOURCE_MISC_TEXTURECUBE	0x4
//
//// DDS_HEADER_DXT10 Flag2
//#define DDS_ALPHA_MODE_UNKNOWN			0x0
//#define DDS_ALPHA_MODE_STRAIGHT			0x1
//#define DDS_ALPHA_MODE_PREMULTIPLIED	0x2
//#define DDS_ALPHA_MODE_OPAQUE			0x3
//#define DDS_ALPHA_MODE_CUSTOM			0x4