#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__
#include "D3DHeader.h"
#include <vector>
#include "Light.h"

class CCamera;
class RenderObject;
#define NUM_TONEMAP_TEXTURES  4       // Number of stages in the 4x4 down-scaling 
#define NUM_BLOOM_TEXTURES    3       // Number of textures used for the bloom
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
	void _renderShadowMap();
	void _renderSurface();
	void _scene_to_sceneScaled();
	void _getTextureCoords( PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest,
		CoordRect* pCoords );
	void _getSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] );
	void _drawFullScreenQuad( float fLeftU, float fTopV, float fRightU, float fBottomV );
	void _measureLuminance();
	void _calculateAdaptation();
	void _sceneScaled_To_BrightPass();
	void _brightSource_ToBloomSource();
	void _getTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect );
	void _getSampleOffsets_GaussBlur5x5( DWORD dwD3DTexWidth,
		DWORD dwD3DTexHeight,
		D3DXVECTOR2* avTexCoordOffset,
		D3DXVECTOR4* avSampleWeight,
		FLOAT fMultiplier );
	//void _starSource_To_BloomSource();
	void _renderBloom();
	void _getSampleOffsets_Bloom( DWORD dwD3DTexSize,
		float afTexCoordOffset[15],
		D3DXVECTOR4* avColorWeight,
		float fDeviation,
		float fMultiplier );
	std::vector<RenderObject*> mRenderObjectList;
	std::vector<Light*>		mLightList;
	CCamera*				g_camera;
	bool					g_usedHDR;
	static D3DRender* m_pInstance;
	D3DRender();
	D3DPRESENT_PARAMETERS		  g_D3dpp;
	LPDIRECT3D9                   g_pD3D        ;  // Direct3D对象指针
	LPDIRECT3DDEVICE9             g_pd3dDevice  ;  // Direct3D设备指针
	LPDIRECT3DTEXTURE9			  g_pShadowMap;
	LPDIRECT3DSURFACE9            g_pDSShadow;
	LPDIRECT3DTEXTURE9			  g_pTexScene;
	LPDIRECT3DTEXTURE9			  g_pTexSceneScaled;
	LPDIRECT3DTEXTURE9			  g_apTexToneMap[NUM_TONEMAP_TEXTURES]; // Log average luminance samples 
	PDIRECT3DTEXTURE9			  g_apTexBloom[NUM_BLOOM_TEXTURES];     // Blooming effect working textures
	LPDIRECT3DTEXTURE9			  g_pTexAdaptedLuminanceLast;
	LPDIRECT3DTEXTURE9			  g_pTexAdaptedLuminanceCur;
	LPDIRECT3DTEXTURE9			  g_pTexBrightPass;
	//LPDIRECT3DTEXTURE9			  g_pTexStarSource;
	LPDIRECT3DTEXTURE9			  g_pTexBloomSource;
	ID3DXEffect*				  g_HDREffect;
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