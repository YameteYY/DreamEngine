#include "D3DRender.h"
#include "../Object/RenderObject.h"
#include "Camera.h"
#include "SpotLight.h"

D3DRender* D3DRender::m_pInstance = 0;
#define MAX_SAMPLES           16      // Maximum number of texture grabs
D3DRender::D3DRender()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_usedHDR = true;
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
	g_D3dpp.BackBufferWidth = d3ddm.Height;
	g_D3dpp.BackBufferHeight = d3ddm.Width;
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

	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth, g_D3dpp.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
		D3DPOOL_DEFAULT, &g_pTexScene, NULL );

	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 4, g_D3dpp.BackBufferHeight / 4,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&g_pTexSceneScaled, NULL );

	ID3DXBuffer* errorBuffer = 0;
	D3DXCreateEffectFromFile(
		g_pd3dDevice,
		"res/hdr.fx",
		0,                // no preprocessor definitions
		0,                // no ID3DXInclude interface
		D3DXSHADER_DEBUG, // compile flags
		0,                // don't share parameters
		&g_HDREffect,
		&errorBuffer);
	if( errorBuffer )
	{
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		SAFE_RELEASE(errorBuffer);
	}

	// For each scale stage, create a texture to hold the intermediate results
	// of the luminance calculation
	for(int i = 0; i < NUM_TONEMAP_TEXTURES; i++ )
	{
		int iSampleLen = 1 << ( 2 * i );

		g_pd3dDevice->CreateTexture( iSampleLen, iSampleLen, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F, D3DPOOL_DEFAULT,
			&g_apTexToneMap[i], NULL );
	}
	g_pd3dDevice->CreateTexture( 1, 1, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F, D3DPOOL_DEFAULT,
		&g_pTexAdaptedLuminanceLast, NULL );
	g_pd3dDevice->CreateTexture( 1, 1, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F, D3DPOOL_DEFAULT,
		&g_pTexAdaptedLuminanceCur, NULL );

	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 4 + 2, g_D3dpp.BackBufferWidth / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexBrightPass, NULL );
	/*
	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 4 + 2, g_D3dpp.BackBufferWidth / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexStarSource, NULL );*/

	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 8 + 2, g_D3dpp.BackBufferWidth / 8 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexBloomSource, NULL );

	for(int i = 0; i < NUM_BLOOM_TEXTURES; i++ )
	{
		g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 8 + 2, g_D3dpp.BackBufferWidth / 8 + 2,
			1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT, &g_apTexBloom[i], NULL );
	}
	return S_OK;
}
void D3DRender::Render()
{
	_renderShadowMap();

	PDIRECT3DSURFACE9 pSurfLDR; // Low dynamic range surface for final output
	PDIRECT3DSURFACE9 pSurfDS;  // Low dynamic range depth stencil surface
	PDIRECT3DSURFACE9 pSurfHDR; // High dynamic range surface to store 
	if(g_usedHDR)
	{
		g_pd3dDevice->GetRenderTarget(0,&pSurfLDR);
		g_pd3dDevice->GetDepthStencilSurface(&pSurfDS);

		g_pTexScene->GetSurfaceLevel(0,&pSurfHDR);

		g_pd3dDevice->SetRenderTarget(0,pSurfHDR);
	}
	_renderSurface();
	if(!g_usedHDR)
		return;
	_scene_to_sceneScaled();

	_measureLuminance();

	_calculateAdaptation();

	_sceneScaled_To_BrightPass();
	_brightSource_ToBloomSource();
	_renderBloom();

	UINT uiPassCount, uiPass;

	g_HDREffect->SetTechnique( "FinalScenePass" );
	g_HDREffect->SetFloat( "g_fMiddleGray", 0.18);

	g_pd3dDevice->SetRenderTarget( 0, pSurfLDR );
	g_pd3dDevice->SetTexture( 0, g_pTexScene ) ;
	g_pd3dDevice->SetTexture( 1, g_apTexBloom[0] );
	g_pd3dDevice->SetTexture( 2, g_pTexAdaptedLuminanceCur ) ;
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) ;
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT ) ;
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR ) ;
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) ;
	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) ;
	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_POINT);


	g_HDREffect->Begin( &uiPassCount, 0 );
	{
		for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
		{
			g_HDREffect->BeginPass( uiPass );

			_drawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

			g_HDREffect->EndPass();
		}
	}
	g_HDREffect->End();
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
void D3DRender::_scene_to_sceneScaled()
{
	PDIRECT3DSURFACE9 pSurfScaledScene = NULL;
	g_pTexSceneScaled->GetSurfaceLevel( 0, &pSurfScaledScene );
	
	g_HDREffect->SetTechnique("DownScale4x4");

	RECT rectSrc;
	rectSrc.left = 0;
	rectSrc.top = 0;
	rectSrc.right = g_D3dpp.BackBufferWidth;
	rectSrc.bottom = g_D3dpp.BackBufferHeight;

	CoordRect coords;
	_getTextureCoords( g_pTexScene, &rectSrc, g_pTexSceneScaled, NULL, &coords );
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	_getSampleOffsets_DownScale4x4(g_D3dpp.BackBufferWidth,g_D3dpp.BackBufferHeight,avSampleOffsets);
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

	g_pd3dDevice->SetRenderTarget( 0, pSurfScaledScene );
	g_pd3dDevice->SetTexture( 0, g_pTexScene );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	UINT uiPassCount, uiPass;
	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV, coords.fRightU,coords.fBottomV);

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
}
void D3DRender::_getTextureCoords( PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords )
{
	D3DSURFACE_DESC desc;
	float tU, tV;

	// Validate arguments
	if( pTexSrc == NULL || pTexDest == NULL || pCoords == NULL )
		return ;

	// Start with a default mapping of the complete source surface to complete 
	// destination surface
	pCoords->fLeftU = 0.0f;
	pCoords->fTopV = 0.0f;
	pCoords->fRightU = 1.0f;
	pCoords->fBottomV = 1.0f;

	// If not using the complete source surface, adjust the coordinates
	if( pRectSrc != NULL )
	{
		// Get destination texture description
		pTexSrc->GetLevelDesc( 0, &desc );

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;

		pCoords->fLeftU += pRectSrc->left * tU;
		pCoords->fTopV += pRectSrc->top * tV;
		pCoords->fRightU -= ( desc.Width - pRectSrc->right ) * tU;
		pCoords->fBottomV -= ( desc.Height - pRectSrc->bottom ) * tV;
	}

	// If not drawing to the complete destination surface, adjust the coordinates
	if( pRectDest != NULL )
	{
		// Get source texture description
		pTexDest->GetLevelDesc( 0, &desc );

		// These delta values are the distance between source texel centers in 
		// texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;

		pCoords->fLeftU -= pRectDest->left * tU;
		pCoords->fTopV -= pRectDest->top * tV;
		pCoords->fRightU += ( desc.Width - pRectDest->right ) * tU;
		pCoords->fBottomV += ( desc.Height - pRectDest->bottom ) * tV;
	}
}
void D3DRender::_getSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
{
	if( NULL == avSampleOffsets )
		return ;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	// Sample from the 16 surrounding points. Since the center point will be in
	// the exact center of 16 texels, a 0.5f offset is needed to specify a texel
	// center.
	int index = 0;
	for( int y = 0; y < 4; y++ )
	{
		for( int x = 0; x < 4; x++ )
		{
			avSampleOffsets[ index ].x = ( x - 1.5f ) * tU;
			avSampleOffsets[ index ].y = ( y - 1.5f ) * tV;

			index++;
		}
	}
}
void D3DRender::_drawFullScreenQuad( float fLeftU, float fTopV, float fRightU, float fBottomV )
{
	D3DSURFACE_DESC dtdsdRT;
	PDIRECT3DSURFACE9 pSurfRT;

	// Acquire render target width and height
	g_pd3dDevice->GetRenderTarget( 0, &pSurfRT );
	pSurfRT->GetDesc( &dtdsdRT );
	pSurfRT->Release();

	// Ensure that we're directly mapping texels to pixels by offset by 0.5
	// For more info see the doc page titled "Directly Mapping Texels to Pixels"
	FLOAT fWidth5 = ( FLOAT )dtdsdRT.Width - 0.5f;
	FLOAT fHeight5 = ( FLOAT )dtdsdRT.Height - 0.5f;

	// Draw the quad
	ScreenVertex svQuad[4];

	svQuad[0].p = D3DXVECTOR4( -0.5f, -0.5f, 0.5f, 1.0f );
	svQuad[0].t = D3DXVECTOR2( fLeftU, fTopV );

	svQuad[1].p = D3DXVECTOR4( fWidth5, -0.5f, 0.5f, 1.0f );
	svQuad[1].t = D3DXVECTOR2( fRightU, fTopV );

	svQuad[2].p = D3DXVECTOR4( -0.5f, fHeight5, 0.5f, 1.0f );
	svQuad[2].t = D3DXVECTOR2( fLeftU, fBottomV );

	svQuad[3].p = D3DXVECTOR4( fWidth5, fHeight5, 0.5f, 1.0f );
	svQuad[3].t = D3DXVECTOR2( fRightU, fBottomV );

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	g_pd3dDevice->SetFVF( ScreenVertex::FVF );
	g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof( ScreenVertex ) );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
}
void D3DRender::_measureLuminance()
{
	UINT uiPassCount, uiPass;
	int i, x, y, index;
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

	DWORD dwCurTexture = NUM_TONEMAP_TEXTURES - 1;
	PDIRECT3DSURFACE9 apSurfToneMap[NUM_TONEMAP_TEXTURES] = {0};

	// Retrieve the tonemap surfaces
	for( i = 0; i < NUM_TONEMAP_TEXTURES; i++ )
	{
		g_apTexToneMap[i]->GetSurfaceLevel( 0, &apSurfToneMap[i] );
	}
	D3DSURFACE_DESC desc;
	g_apTexToneMap[dwCurTexture]->GetLevelDesc( 0, &desc );

	// Initialize the sample offsets for the initial luminance pass.
	float tU, tV;
	tU = 1.0f / ( 3.0f * desc.Width );
	tV = 1.0f / ( 3.0f * desc.Height );

	index = 0;
	for( x = -1; x <= 1; x++ )
	{
		for( y = -1; y <= 1; y++ )
		{
			avSampleOffsets[index].x = x * tU;
			avSampleOffsets[index].y = y * tV;

			index++;
		}
	}

	g_HDREffect->SetTechnique("SampleAvgLum");
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

	g_pd3dDevice->SetRenderTarget( 0, apSurfToneMap[dwCurTexture] );
	g_pd3dDevice->SetTexture( 0, g_pTexSceneScaled );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_HDREffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();

	dwCurTexture--;
	while( dwCurTexture > 0 )
	{
		g_apTexToneMap[dwCurTexture + 1]->GetLevelDesc( 0, &desc );
		_getSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );


		// Each of these passes continue to scale down the log of average
		// luminance texture created above, storing intermediate results in 
		// g_apTexToneMap[1] through g_apTexToneMap[NUM_TONEMAP_TEXTURES-1].
		g_HDREffect->SetTechnique( "ResampleAvgLum" );
		g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

		g_pd3dDevice->SetRenderTarget( 0, apSurfToneMap[dwCurTexture] );
		g_pd3dDevice->SetTexture( 0, g_apTexToneMap[dwCurTexture + 1] );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );


		g_HDREffect->Begin( &uiPassCount, 0 );

		for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
		{
			g_HDREffect->BeginPass( uiPass );

			// Draw a fullscreen quad to sample the RT
			_drawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

			g_HDREffect->EndPass();
		}

		g_HDREffect->End();
		dwCurTexture--;
	}
	g_apTexToneMap[1]->GetLevelDesc( 0, &desc );
	_getSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );

	g_HDREffect->SetTechnique( "ResampleAvgLumExp" );
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

	g_pd3dDevice->SetRenderTarget( 0, apSurfToneMap[0] );
	g_pd3dDevice->SetTexture( 0, g_apTexToneMap[1] );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );


	g_HDREffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();

	for( i = 0; i < NUM_TONEMAP_TEXTURES; i++ )
	{
		SAFE_RELEASE( apSurfToneMap[i] );
	}
}
void D3DRender::_calculateAdaptation()
{
	UINT uiPass, uiPassCount;
	// Swap current & last luminance
	PDIRECT3DTEXTURE9 pTexSwap = g_pTexAdaptedLuminanceLast;
	g_pTexAdaptedLuminanceLast = g_pTexAdaptedLuminanceCur;
	g_pTexAdaptedLuminanceCur = pTexSwap;

	PDIRECT3DSURFACE9 pSurfAdaptedLum = NULL;
	g_pTexAdaptedLuminanceCur->GetSurfaceLevel( 0, &pSurfAdaptedLum);

	// This simulates the light adaptation that occurs when moving from a 
	// dark area to a bright area, or vice versa. The g_pTexAdaptedLuminance
	// texture stores a single texel cooresponding to the user's adapted 
	// level.
	g_HDREffect->SetTechnique( "CalculateAdaptedLum" );
	g_HDREffect->SetFloat( "g_fElapsedTime", 0.05);

	g_pd3dDevice->SetRenderTarget( 0, pSurfAdaptedLum );
	g_pd3dDevice->SetTexture( 0, g_pTexAdaptedLuminanceLast );
	g_pd3dDevice->SetTexture( 1, g_apTexToneMap[0] );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );


	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();


	SAFE_RELEASE( pSurfAdaptedLum );
}
void D3DRender::_sceneScaled_To_BrightPass()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];


	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBrightPass;
	g_pTexBrightPass->GetSurfaceLevel( 0, &pSurfBrightPass );


	D3DSURFACE_DESC desc;
	g_pTexSceneScaled->GetLevelDesc( 0, &desc );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	_getTextureRect( g_pTexSceneScaled, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	_getTextureRect( g_pTexBrightPass, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	_getTextureCoords( g_pTexSceneScaled, &rectSrc, g_pTexBrightPass, &rectDest, &coords );

	// The bright-pass filter removes everything from the scene except lights and
	// bright reflections
	g_HDREffect->SetTechnique( "BrightPassFilter" );

	g_pd3dDevice->SetRenderTarget( 0, pSurfBrightPass );
	g_pd3dDevice->SetTexture( 0, g_pTexSceneScaled );
	g_pd3dDevice->SetTexture( 1, g_pTexAdaptedLuminanceCur );
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	g_pd3dDevice->SetScissorRect( &rectDest );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	UINT uiPass, uiPassCount;
	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	SAFE_RELEASE( pSurfBrightPass );
}
//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2
// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_DownScale2x2( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
{
	if( NULL == avSampleOffsets )
		return E_INVALIDARG;

	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;

	// Sample from the 4 surrounding points. Since the center point will be in
	// the exact center of 4 texels, a 0.5f offset is needed to specify a texel
	// center.
	int index = 0;
	for( int y = 0; y < 2; y++ )
	{
		for( int x = 0; x < 2; x++ )
		{
			avSampleOffsets[ index ].x = ( x - 0.5f ) * tU;
			avSampleOffsets[ index ].y = ( y - 0.5f ) * tV;

			index++;
		}
	}

	return S_OK;
}
void D3DRender::_brightSource_ToBloomSource()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBloomSource;
	g_pTexBloomSource->GetSurfaceLevel( 0, &pSurfBloomSource );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	_getTextureRect( g_pTexBrightPass, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	_getTextureRect( g_pTexBloomSource, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	_getTextureCoords( g_pTexBrightPass, &rectSrc, g_pTexBloomSource, &rectDest, &coords );

	// Get the sample offsets used within the pixel shader
	D3DSURFACE_DESC desc;
	g_pTexBrightPass->GetLevelDesc( 0, &desc );

	GetSampleOffsets_DownScale2x2( desc.Width, desc.Height, avSampleOffsets );
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

	// Create an exact 1/2 x 1/2 copy of the source texture
	g_HDREffect->SetTechnique( "DownScale2x2" );

	g_pd3dDevice->SetRenderTarget( 0, pSurfBloomSource );
	g_pd3dDevice->SetTexture( 0, g_pTexBrightPass );
	g_pd3dDevice->SetScissorRect( &rectDest );
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	UINT uiPassCount, uiPass;
	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	SAFE_RELEASE( pSurfBloomSource );
}
void D3DRender::_getTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect )
{
	if( pTexture == NULL || pRect == NULL )
		return ;

	D3DSURFACE_DESC desc;
	pTexture->GetLevelDesc( 0, &desc );

	pRect->left = 0;
	pRect->top = 0;
	pRect->right = desc.Width;
	pRect->bottom = desc.Height;
}
//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_GaussBlur5x5
// Desc: Get the texture coordinate offsets to be used inside the GaussBlur5x5
//       pixel shader.
//-----------------------------------------------------------------------------
float GaussianDistribution( float x, float y, float rho )
{
	float g = 1.0f / sqrtf( 2.0f * D3DX_PI * rho * rho );
	g *= expf( -( x * x + y * y ) / ( 2 * rho * rho ) );

	return g;
}
void D3DRender::_getSampleOffsets_GaussBlur5x5( DWORD dwD3DTexWidth,
	DWORD dwD3DTexHeight,
	D3DXVECTOR2* avTexCoordOffset,
	D3DXVECTOR4* avSampleWeight,
	FLOAT fMultiplier )
{
	float tu = 1.0f / ( float )dwD3DTexWidth;
	float tv = 1.0f / ( float )dwD3DTexHeight;

	D3DXVECTOR4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );

	float totalWeight = 0.0f;
	int index = 0;
	for( int x = -2; x <= 2; x++ )
	{
		for( int y = -2; y <= 2; y++ )
		{
			// Exclude pixels with a block distance greater than 2. This will
			// create a kernel which approximates a 5x5 kernel using only 13
			// sample points instead of 25; this is necessary since 2.0 shaders
			// only support 16 texture grabs.
			if( abs( x ) + abs( y ) > 2 )
				continue;

			// Get the unscaled Gaussian intensity for this offset
			avTexCoordOffset[index] = D3DXVECTOR2( x * tu, y * tv );
			avSampleWeight[index] = vWhite * GaussianDistribution( ( float )x, ( float )y, 1.0f );
			totalWeight += avSampleWeight[index].x;

			index++;
		}
	}

	// Divide the current weight by the total weight of all the samples; Gaussian
	// blur kernels add to 1.0f to ensure that the intensity of the image isn't
	// changed when the blur occurs. An optional multiplier variable is used to
	// add or remove image intensity during the blur.
	for( int i = 0; i < index; i++ )
	{
		avSampleWeight[i] /= totalWeight;
		avSampleWeight[i] *= fMultiplier;
	}
}
/*
void D3DRender::_starSource_To_BloomSource()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBloomSource;
	g_pTexBloomSource->GetSurfaceLevel( 0, &pSurfBloomSource );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	_getTextureRect( g_pTexStarSource, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	_getTextureRect( g_pTexBloomSource, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	_getTextureCoords( g_pTexStarSource, &rectSrc, g_pTexBloomSource, &rectDest, &coords );

	// Get the sample offsets used within the pixel shader
	D3DSURFACE_DESC desc;
	g_pTexBrightPass->GetLevelDesc( 0, &desc );

	GetSampleOffsets_DownScale2x2( desc.Width, desc.Height, avSampleOffsets );
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );

	// Create an exact 1/2 x 1/2 copy of the source texture
	g_HDREffect->SetTechnique( "DownScale2x2" );

	g_pd3dDevice->SetRenderTarget( 0, pSurfBloomSource );
	g_pd3dDevice->SetTexture( 0, g_pTexStarSource );
	g_pd3dDevice->SetScissorRect( &rectDest );
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	UINT uiPassCount, uiPass;
	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	SAFE_RELEASE( pSurfBloomSource );
}*/
void D3DRender::_renderBloom()
{
	UINT uiPassCount, uiPass;
	int i = 0;


	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	FLOAT afSampleOffsets[MAX_SAMPLES];
	D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];

	PDIRECT3DSURFACE9 pSurfScaledHDR;
	g_pTexSceneScaled->GetSurfaceLevel( 0, &pSurfScaledHDR );

	PDIRECT3DSURFACE9 pSurfBloom;
	g_apTexBloom[0]->GetSurfaceLevel( 0, &pSurfBloom );

	PDIRECT3DSURFACE9 pSurfHDR;
	g_pTexScene->GetSurfaceLevel( 0, &pSurfHDR );

	PDIRECT3DSURFACE9 pSurfTempBloom;
	g_apTexBloom[1]->GetSurfaceLevel( 0, &pSurfTempBloom );

	PDIRECT3DSURFACE9 pSurfBloomSource;
	g_apTexBloom[2]->GetSurfaceLevel( 0, &pSurfBloomSource );

	// Clear the bloom texture
	g_pd3dDevice->ColorFill( pSurfBloom, NULL, D3DCOLOR_ARGB( 0, 0, 0, 0 ) );


	RECT rectSrc;
	_getTextureRect( g_pTexBloomSource, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	RECT rectDest;
	_getTextureRect( g_apTexBloom[2], &rectDest );
	InflateRect( &rectDest, -1, -1 );

	CoordRect coords;
	_getTextureCoords( g_pTexBloomSource, &rectSrc, g_apTexBloom[2], &rectDest, &coords );

	D3DSURFACE_DESC desc;
	g_pTexBloomSource->GetLevelDesc( 0, &desc );


	g_HDREffect->SetTechnique( "GaussBlur5x5" );

	_getSampleOffsets_GaussBlur5x5( desc.Width, desc.Height, avSampleOffsets, avSampleWeights, 1.0f );

	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );
	g_HDREffect->SetValue( "g_avSampleWeights", avSampleWeights, sizeof( avSampleWeights ) );

	g_pd3dDevice->SetRenderTarget( 0, pSurfBloomSource );
	g_pd3dDevice->SetTexture( 0, g_pTexBloomSource );
	g_pd3dDevice->SetScissorRect( &rectDest );
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );


	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	g_apTexBloom[2]->GetLevelDesc( 0, &desc );

	_getSampleOffsets_Bloom( desc.Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
	for( i = 0; i < MAX_SAMPLES; i++ )
	{
		avSampleOffsets[i] = D3DXVECTOR2( afSampleOffsets[i], 0.0f );
	}


	g_HDREffect->SetTechnique( "Bloom" );
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );
	g_HDREffect->SetValue( "g_avSampleWeights", avSampleWeights, sizeof( avSampleWeights ) );

	g_pd3dDevice->SetRenderTarget( 0, pSurfTempBloom );
	g_pd3dDevice->SetTexture( 0, g_apTexBloom[2] );
	g_pd3dDevice->SetScissorRect( &rectDest );
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );


	g_HDREffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );


	g_apTexBloom[1]->GetLevelDesc( 0, &desc );

	_getSampleOffsets_Bloom( desc.Height, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
	for( i = 0; i < MAX_SAMPLES; i++ )
	{
		avSampleOffsets[i] = D3DXVECTOR2( 0.0f, afSampleOffsets[i] );
	}


	_getTextureRect( g_apTexBloom[1], &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	_getTextureCoords( g_apTexBloom[1], &rectSrc, g_apTexBloom[0], NULL, &coords );


	g_HDREffect->SetTechnique( "Bloom" );
	g_HDREffect->SetValue( "g_avSampleOffsets", avSampleOffsets, sizeof( avSampleOffsets ) );
	g_HDREffect->SetValue( "g_avSampleWeights", avSampleWeights, sizeof( avSampleWeights ) );

	g_pd3dDevice->SetRenderTarget( 0, pSurfBloom );
	g_pd3dDevice->SetTexture( 0, g_apTexBloom[1] );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );


	g_HDREffect->Begin( &uiPassCount, 0 );

	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		_drawFullScreenQuad( coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();

	SAFE_RELEASE( pSurfBloomSource );
	SAFE_RELEASE( pSurfTempBloom );
	SAFE_RELEASE( pSurfBloom );
	SAFE_RELEASE( pSurfHDR );
	SAFE_RELEASE( pSurfScaledHDR );
}
void D3DRender::_getSampleOffsets_Bloom( DWORD dwD3DTexSize,
	float afTexCoordOffset[15],
	D3DXVECTOR4* avColorWeight,
	float fDeviation,
	float fMultiplier )
{
	int i = 0;
	float tu = 1.0f / ( float )dwD3DTexSize;

	// Fill the center texel
	float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
	avColorWeight[0] = D3DXVECTOR4( weight, weight, weight, 1.0f );

	afTexCoordOffset[0] = 0.0f;

	// Fill the first half
	for( i = 1; i < 8; i++ )
	{
		// Get the Gaussian intensity for this offset
		weight = fMultiplier * GaussianDistribution( ( float )i, 0, fDeviation );
		afTexCoordOffset[i] = i * tu;

		avColorWeight[i] = D3DXVECTOR4( weight, weight, weight, 1.0f );
	}

	// Mirror to the second half
	for( i = 8; i < 15; i++ )
	{
		avColorWeight[i] = avColorWeight[i - 7];
		afTexCoordOffset[i] = -afTexCoordOffset[i - 7];
	}
}