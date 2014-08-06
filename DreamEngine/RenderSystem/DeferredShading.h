#ifndef __DEFERREDSHADING_H__
#define __DEFERREDSHADING_H__
#include "D3DHeader.h"
#include "../Object/RenderObject.h"
#include <vector>

class Light;
class DeferredShading
{
public:
	
	void Init(D3DDISPLAYMODE d3ddm);
	void RenderGBuffer(std::vector<RenderObject*> renderObjList);
	void RenderLight(std::vector<Light*>* lightList);
	void RenderSSAO();
private:
	void ClearTexture(IDirect3DTexture9* pd3dTexture,D3DCOLOR xColor);
	void SetRenderTarget(int iRenderTargetIdx,LPDIRECT3DTEXTURE9 pd3dRenderTargetTexture);
	float						  mWindowWidth;
	float						  mWindowHeight;
	LPDIRECT3DDEVICE9             g_pd3dDevice  ;  // Direct3D…Ë±∏÷∏’Î
	LPDIRECT3DTEXTURE9			  mDiffuseTex;
	LPDIRECT3DTEXTURE9			  mPositionTex;
	LPDIRECT3DTEXTURE9			  mNormalTex;
	LPDIRECT3DTEXTURE9			  mLightPassTex;
	ID3DXEffect*				  mDeferredEffect;
	bool						  mUsedSSAO;
};

#endif