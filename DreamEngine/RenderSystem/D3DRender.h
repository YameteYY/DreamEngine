#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__
#include "D3DHeader.h"
#include <vector>
#include "Light.h"

class CCamera;
class RenderObject;
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
private:
	std::vector<RenderObject*> mRenderObjectList;
	std::vector<Light*>		mLightList;
	CCamera*				g_camera;
	static D3DRender* m_pInstance;
	D3DRender();
	LPDIRECT3D9                   g_pD3D        ;  // Direct3D对象指针
	LPDIRECT3DDEVICE9             g_pd3dDevice  ;  // Direct3D设备指针
};
inline void D3DRender::AddRenderObject(RenderObject* obj)
{
	mRenderObjectList.push_back(obj);
}
inline void D3DRender::AddLight(Light* light)
{
	mLightList.push_back(light);
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


#endif