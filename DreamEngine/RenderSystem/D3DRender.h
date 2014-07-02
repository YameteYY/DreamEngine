#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__
#include "D3DHeader.h"

class D3DRender
{
public:
	D3DRender();
	~D3DRender();
	HRESULT  InitD3D( HWND hWnd);
	void Render();
private:
	LPDIRECT3D9                   g_pD3D        ;  // Direct3D对象指针
	LPDIRECT3DDEVICE9             g_pd3dDevice  ;  // Direct3D设备指针
};


#endif