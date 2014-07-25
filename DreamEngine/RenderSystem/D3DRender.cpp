#include "D3DRender.h"
#include "../Object/RenderObject.h"
#include "Camera.h"
#include "SpotLight.h"
#include "DeferredShading.h"

D3DRender* D3DRender::m_pInstance = 0;
D3DRender::D3DRender()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_usedHDR = false;
	g_deferredShading = true;
}
D3DRender::~D3DRender()
{
	SAFE_RELEASE(g_pD3D);
	SAFE_RELEASE(g_pd3dDevice);
}
HRESULT  D3DRender::InitD3D( HWND hWnd)
{
	if( NULL == (g_pD3D = Direct3DCreate9( D3D_SDK_VERSION)) )
		return E_FAIL;
	D3DDISPLAYMODE d3ddm;

	if( FAILED( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT,
			&d3ddm)) )
	{
		return E_FAIL;
	}

	ZeroMemory(&g_D3dpp,sizeof(g_D3dpp));

	g_D3dpp.Windowed = TRUE;
	g_D3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_D3dpp.BackBufferFormat = d3ddm.Format;
	g_D3dpp.BackBufferWidth = d3ddm.Width;
	g_D3dpp.BackBufferHeight = d3ddm.Height;
	g_D3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	g_D3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	g_D3dpp.EnableAutoDepthStencil = true;
	g_D3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if(FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,&g_D3dpp,&g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// Create the shadow map texture
	g_pd3dDevice->CreateTexture( ShadowMap_SIZE, ShadowMap_SIZE,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F,
		D3DPOOL_DEFAULT,
		&g_pShadowMap,
		NULL );

	g_pd3dDevice->CreateDepthStencilSurface( ShadowMap_SIZE,
		ShadowMap_SIZE,
		g_D3dpp.AutoDepthStencilFormat,
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		&g_pDSShadow,
		NULL);

	mHDRLightPostEffect = new HDRLight();
	mHDRLightPostEffect->Init(g_pd3dDevice,g_D3dpp);
	mDeferredShading = new DeferredShading();
	mDeferredShading->Init(d3ddm);
	return S_OK;
}
void D3DRender::Render()
{
	if( GetKeyState('H') & 0x8000 )
	{
		g_deferredShading = true;
	}
	if( GetKeyState('J') & 0x8000 )
	{
		g_deferredShading = false;
	}
	_renderShadowMap();

	if(g_usedHDR)
	{
		mHDRLightPostEffect->SetSurfHDR();
	}
	if(!g_deferredShading)
	{
		_renderSurface();
	}
	else
	{
		mDeferredShading->RenderGBuffer(mRenderObjectList);
		mDeferredShading->RenderLight(&mLightList);
	}
	if(!g_usedHDR)
		return;
	mHDRLightPostEffect->Render();
}
void D3DRender::_renderShadowMap()
{
	LPDIRECT3DSURFACE9 pOldRT = NULL;
	g_pd3dDevice->GetRenderTarget(0,&pOldRT);
	LPDIRECT3DSURFACE9 pShadowSurf;
	if( SUCCEEDED( g_pShadowMap->GetSurfaceLevel( 0, &pShadowSurf ) ) )
	{
		g_pd3dDevice->SetRenderTarget( 0, pShadowSurf );
		SAFE_RELEASE( pShadowSurf );
	}
	LPDIRECT3DSURFACE9 pOldDS = NULL;
	if( SUCCEEDED( g_pd3dDevice->GetDepthStencilSurface( &pOldDS ) ) )
		g_pd3dDevice->SetDepthStencilSurface( g_pDSShadow );

	{
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
			0 );
		g_pd3dDevice->BeginScene();
		ID3DXEffect* effect = NULL;
		int len = mRenderObjectList.size();
		for(int i=0;i<len;i++)
		{
			effect = mRenderObjectList[i]->GetEffect();
			mLightList[0]->SetShaderParam(effect);
			mRenderObjectList[i]->Render(Shadow);
		}
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present(0,0,0,0);
	}

	if( pOldDS )
	{
		g_pd3dDevice->SetDepthStencilSurface( pOldDS );
		pOldDS->Release();
	}
	g_pd3dDevice->SetRenderTarget( 0, pOldRT );
	SAFE_RELEASE( pOldRT );
}
void D3DRender::_renderSurface()
{
	const D3DXMATRIX *mProj = g_camera->GetProjTrans();
	const D3DXMATRIX *mView = g_camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);

	const D3DXVECTOR3& eyePos = g_camera->GetEyePos();

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
		0 );
	g_pd3dDevice->BeginScene();
	ID3DXEffect* effect = NULL;
	int len = mRenderObjectList.size();
	for(int i=0;i<len;i++)
	{
		effect = mRenderObjectList[i]->GetEffect();
		
		effect->SetVector("g_EyePos",&D3DXVECTOR4(eyePos.x,eyePos.y,eyePos.z,1.0f) );
		effect->SetMatrix("g_mViewProjection",&vp);
		effect->SetTexture("ShadowMap",g_pShadowMap);
		for(int j=0;j<mLightList.size();j++)
			mLightList[j]->SetShaderParam(effect);
		mRenderObjectList[i]->Render(Surface);
	}
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(0,0,0,0);
}