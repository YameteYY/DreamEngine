#include "DeferredShading.h"
#include "D3DRender.h"
#include "Math.h"


void DeferredShading::Init(D3DDISPLAYMODE d3ddm)
{
	mUsedSSAO = true;
	mWindowHeight = d3ddm.Height;
	mWindowWidth = d3ddm.Width;

	g_pd3dDevice = D3DRender::Instance()->GetDevice();
	ID3DXBuffer* errorBuffer = 0;
	D3DXCreateEffectFromFile(
		g_pd3dDevice,
		"res/deferred.fx",
		0,                // no preprocessor definitions
		0,                // no ID3DXInclude interface
		D3DXSHADER_DEBUG, // compile flags
		0,                // don't share parameters
		&mDeferredEffect,
		&errorBuffer);
	if( errorBuffer )
	{
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		SAFE_RELEASE(errorBuffer);
	}

	g_pd3dDevice->CreateTexture( mWindowWidth, mWindowHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F,
		D3DPOOL_DEFAULT,
		&mPositionTex,
		NULL );

	g_pd3dDevice->CreateTexture( mWindowWidth, mWindowHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F,
		D3DPOOL_DEFAULT,
		&mNormalTex,
		NULL );

	g_pd3dDevice->CreateTexture( mWindowWidth, mWindowHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&mDiffuseTex,
		NULL );

	g_pd3dDevice->CreateTexture( mWindowWidth, mWindowHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&mLightPassTex,
		NULL );
}
void DeferredShading::ClearTexture(IDirect3DTexture9* pd3dTexture,D3DCOLOR xColor)
{
	IDirect3DSurface9* pd3dSurface;
	pd3dTexture->GetSurfaceLevel(0,&pd3dSurface);
	g_pd3dDevice->ColorFill(pd3dSurface,NULL,xColor);
	pd3dSurface->Release();
}

void DeferredShading::SetRenderTarget(int iRenderTargetIdx,LPDIRECT3DTEXTURE9 pd3dRenderTargetTexture)
{
	IDirect3DSurface9* pd3dSurface;
	pd3dRenderTargetTexture->GetSurfaceLevel(0,&pd3dSurface);
	g_pd3dDevice->SetRenderTarget(iRenderTargetIdx,pd3dSurface);
	pd3dSurface->Release();
}
void DeferredShading::RenderGBuffer(std::vector<RenderObject*> renderObjList)
{
	LPDIRECT3DSURFACE9 pOldRT[3] = {0};
	g_pd3dDevice->GetRenderTarget(0,&pOldRT[0]);
	g_pd3dDevice->GetRenderTarget(1,&pOldRT[1]);
	g_pd3dDevice->GetRenderTarget(2,&pOldRT[2]);
	PDIRECT3DSURFACE9 pDiffuseTex = NULL;
	PDIRECT3DSURFACE9 pNormalTex = NULL;
	PDIRECT3DSURFACE9 pPositionTex = NULL;
	mDiffuseTex->GetSurfaceLevel(0,&pDiffuseTex);
	mNormalTex->GetSurfaceLevel(0,&pNormalTex);
	mPositionTex->GetSurfaceLevel(0,&pPositionTex);

	g_pd3dDevice->SetRenderTarget(0,pDiffuseTex);
	g_pd3dDevice->SetRenderTarget(1,pNormalTex);
	g_pd3dDevice->SetRenderTarget(2,pPositionTex);

	CCamera* g_camera = D3DRender::Instance()->GetCamera();
	const D3DXMATRIX *mProj = g_camera->GetProjTrans();
	const D3DXMATRIX *mView = g_camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
		0 );
	g_pd3dDevice->BeginScene();
	mDeferredEffect->SetTechnique("GBufferTechnique");
	ID3DXEffect* effect = NULL;
	int len = renderObjList.size();
	for(int i=0;i<len;i++)
	{
		mDeferredEffect->SetMatrix("g_mViewProjection",&vp);
		effect = renderObjList[i]->GetEffect();
		renderObjList[i]->SetEffect(mDeferredEffect);
		renderObjList[i]->Render(GBuffer);
		renderObjList[i]->SetEffect(effect);
	}
	//std::vector<Light*>* lightList = D3DRender::Instance()->GetLightList();
	//(*lightList)[0]->Render(mDeferredEffect);

	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(0,0,0,0);

	g_pd3dDevice->SetRenderTarget( 0, pOldRT[0] );
	g_pd3dDevice->SetRenderTarget( 1, pOldRT[1] );
	g_pd3dDevice->SetRenderTarget( 2, pOldRT[2] );

	SAFE_RELEASE( pOldRT[0] );
	SAFE_RELEASE( pOldRT[1] );
	SAFE_RELEASE( pOldRT[2] );
	SAFE_RELEASE(pDiffuseTex);
	SAFE_RELEASE(pNormalTex);
	SAFE_RELEASE(pPositionTex);
}
void DeferredShading::RenderLight(std::vector<Light*>* lightList)
{
	LPDIRECT3DSURFACE9 pOldRT = 0;
	PDIRECT3DSURFACE9 pLightPassTex = NULL;
	if(mUsedSSAO)
	{
		g_pd3dDevice->GetRenderTarget(0,&pOldRT);

		mLightPassTex->GetSurfaceLevel(0,&pLightPassTex);
		g_pd3dDevice->SetRenderTarget(0,pLightPassTex);
	}
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f ), 1.0f,
		0 );
	g_pd3dDevice->BeginScene();
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);

	CCamera* g_camera = D3DRender::Instance()->GetCamera();
	const D3DXVECTOR3& eyePos = g_camera->GetEyePos();
	std::vector<LPDIRECT3DTEXTURE9>* shadowMapList = D3DRender::Instance()->GetShadowMap();
	mDeferredEffect->SetVector("g_EyePos",&D3DXVECTOR4(eyePos.x,eyePos.y,eyePos.z,1.0f) );
	mDeferredEffect->SetTexture("DiffuseMap",mDiffuseTex);
	mDeferredEffect->SetTexture("NormalMap",mNormalTex);
	mDeferredEffect->SetTexture("PositionMap",mPositionTex);


	mDeferredEffect->SetTechnique("AmbientTechnique");
	UINT uiPassCount, uiPass;
	mDeferredEffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		mDeferredEffect->BeginPass(uiPass);
		DrawFullScreenQuad(g_pd3dDevice,0,0,1,1);
		mDeferredEffect->EndPass();
	}
	mDeferredEffect->End();
	mDeferredEffect->SetTechnique("LightTechnique");
	const D3DXMATRIX *mProj = g_camera->GetProjTrans();
	const D3DXMATRIX *mView = g_camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);
	mDeferredEffect->SetMatrix("g_mViewProjection",&vp);
	mDeferredEffect->CommitChanges();
	//g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
	g_pd3dDevice ->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	int len = lightList->size();
	for(int i=0;i<len;i++)
	{
		mDeferredEffect->SetTexture("ShadowMap",(*shadowMapList)[i]);

		(*lightList)[i]->SetShaderParam(mDeferredEffect);
		mDeferredEffect->CommitChanges();
		(*lightList)[i]->Render(mDeferredEffect);
		/*
		UINT uiPassCount, uiPass;
		mDeferredEffect->Begin( &uiPassCount, 0 );
		for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
		{
			mDeferredEffect->BeginPass(uiPass);
			(*lightList)[i]->SetShaderParam(mDeferredEffect);
			mDeferredEffect->CommitChanges();
			DrawFullScreenQuad(g_pd3dDevice,0,0,1,1);
			mDeferredEffect->EndPass();
		}
		mDeferredEffect->End();*/
	}
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,false);
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(0,0,0,0);
	g_pd3dDevice ->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS);
	if(mUsedSSAO)
	{
		g_pd3dDevice->SetRenderTarget( 0, pOldRT);
		SAFE_RELEASE( pOldRT);
		SAFE_RELEASE(pLightPassTex);
	}
}
void DeferredShading::RenderSSAO()
{
	if(!mUsedSSAO)
		return;
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.0f, 0.0f, 0.0f ), 1.0f,
		0 );
	g_pd3dDevice->BeginScene();

	mDeferredEffect->SetTexture("DiffuseMap",mLightPassTex);
	mDeferredEffect->SetTexture("NormalMap",mNormalTex);
	mDeferredEffect->SetTexture("PositionMap",mPositionTex);
	mDeferredEffect->SetTechnique("SSAOTechnique");
	mDeferredEffect->CommitChanges();

	UINT uiPassCount, uiPass;
	mDeferredEffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		mDeferredEffect->BeginPass(uiPass);
		DrawFullScreenQuad(g_pd3dDevice,0,0,1,1);
		mDeferredEffect->EndPass();
	}
	mDeferredEffect->End();
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(0,0,0,0);
}