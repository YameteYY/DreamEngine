#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__
#include "D3DHeader.h"
#include <vector>
#include "Light.h"
#include "HDRLight.h"

class CCamera;
class RenderObject;
class DeferredShading;
class D3DRender
{
public:
	static D3DRender* Instance()
	{
		if(0 == m_pInstance)
			m_pInstance = new D3DRender();
		return m_pInstance;
	}
	~D3DRender();
	HRESULT  InitD3D( HWND hWnd);
	void Render();
	LPDIRECT3DDEVICE9 GetDevice();
	void SetCamera(CCamera* camera);
	CCamera* GetCamera();
	std::vector<Light*>* GetLightList();
	void AddRenderObject(RenderObject* obj);
	void AddLight(Light* light);
	std::vector<LPDIRECT3DTEXTURE9>* GetShadowMap();
private:
	void _renderShadowMap();
	void _renderSurface();

	HDRLight*				mHDRLightPostEffect;
	DeferredShading*		mDeferredShading;
	std::vector<RenderObject*> mRenderObjectList;
	std::vector<Light*>		mLightList;
	CCamera*				g_camera;
	bool					g_usedHDR;
	bool					g_deferredShading;
	static D3DRender* m_pInstance;
	D3DRender();
	D3DPRESENT_PARAMETERS						g_D3dpp;
	LPDIRECT3D9									g_pD3D        ;  // Direct3D对象指针
	LPDIRECT3DDEVICE9							g_pd3dDevice  ;  // Direct3D设备指针
	std::vector<LPDIRECT3DTEXTURE9>				g_pShadowMapList;
	LPDIRECT3DSURFACE9							g_pDSShadow;
};
inline void D3DRender::AddRenderObject(RenderObject* obj)
{
	mRenderObjectList.push_back(obj);
}
inline LPDIRECT3DDEVICE9 D3DRender::GetDevice()
{
	return g_pd3dDevice;
}
inline void D3DRender::SetCamera(CCamera* camera)
{
	g_camera = camera;
}
inline CCamera* D3DRender::GetCamera()
{
	return g_camera;
}
inline std::vector<Light*>* D3DRender::GetLightList()
{
	return &mLightList;
}
inline std::vector<LPDIRECT3DTEXTURE9>* D3DRender::GetShadowMap()
{
	return &g_pShadowMapList;
}


#endif