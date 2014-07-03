#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__
#include "D3DHeader.h"
#include <vector>

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
	void AddRenderObject(RenderObject* obj);
private:
	std::vector<RenderObject*> mRenderObjectList;
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


#endif