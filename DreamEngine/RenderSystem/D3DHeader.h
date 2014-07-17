#ifndef __D3DHEADER_H__
#define __D3DHEADER_H__

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define SAFE_DELETE(p)         {if(p) {delete (p);    (p) = NULL;}}
#define SAFE_DELETE_ARRAY(p)   {if(p) {delete[] (p);  (p) = NULL;}}
#define SAFE_RELEASE(p)        {if(p) {(p)->Release();(p) = NULL;}}


enum RenderType
{
	Surface,
	Shadow
};

#define ShadowMap_SIZE 512
#endif