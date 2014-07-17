#include "D3DRender.h"
#include "../Object/RenderObject.h"
#include "Camera.h"

D3DRender* D3DRender::m_pInstance = 0;
D3DRender::D3DRender()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
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

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	d3dpp.BackBufferWidth = d3ddm.Height;
	d3dpp.BackBufferHeight = d3ddm.Width;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if(FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&g_pd3dDevice)))
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
		d3dpp.AutoDepthStencilFormat,
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		&g_pDSShadow,
		NULL);
	return S_OK;
}
void D3DRender::Render()
{
	const D3DXMATRIX *mProj = g_camera->GetProjTrans();
	const D3DXMATRIX *mView = g_camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);

	const D3DXMATRIX *mLightProj = mLightList[0]->GetProjTrans();
	const D3DXMATRIX *mLightView = mLightList[0]->GetViewTrans();
	D3DXMATRIX LightVp;
	D3DXMatrixMultiply(&LightVp,mLightView,mLightProj);
	
	const D3DXVECTOR3& eyePos = g_camera->GetEyePos();
	const D3DXVECTOR3& lightdir = mLightList[0]->GetDirection();
	const D3DXVECTOR3& lightpos = mLightList[0]->GetPosition();	
	
	
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
			effect->SetMatrix("g_mLightVP",&LightVp);
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


	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
		0 );
	g_pd3dDevice->BeginScene();
	ID3DXEffect* effect = NULL;
	int len = mRenderObjectList.size();
	for(int i=0;i<len;i++)
	{
		effect = mRenderObjectList[i]->GetEffect();
		effect->SetVector("g_LightDir",&D3DXVECTOR4(lightdir.x,lightdir.y,lightdir.z,0.0));
		effect->SetVector("g_vLightPos",&D3DXVECTOR4(lightpos.x,lightpos.y,lightpos.z,0.0));
		effect->SetVector("g_EyePos",&D3DXVECTOR4(eyePos.x,eyePos.y,eyePos.z,1.0f) );
		effect->SetMatrix("g_mViewProjection",&vp);
		effect->SetMatrix("g_mLightVP",&LightVp);
		effect->SetTexture("ShadowMap",g_pShadowMap);
		effect->SetFloat("g_fCosTheta",mLightList[0]->GetCosTheta());
		mRenderObjectList[i]->Render(Surface);
	}
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(0,0,0,0);
}