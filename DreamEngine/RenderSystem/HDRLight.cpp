#include "HDRLight.h"
#include "Math.h"
#define MAX_SAMPLES           16      // Maximum number of texture grabs

void HDRLight::Init(LPDIRECT3DDEVICE9 Device,D3DPRESENT_PARAMETERS d3dpp)
{
	g_pd3dDevice = Device;
	g_D3dpp = d3dpp;

	g_pd3dDevice->CreateTexture( d3dpp.BackBufferWidth, d3dpp.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
		D3DPOOL_DEFAULT, &g_pTexScene, NULL );

	g_pd3dDevice->CreateTexture( d3dpp.BackBufferWidth / 4, d3dpp.BackBufferHeight / 4,
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

	g_pd3dDevice->CreateTexture( d3dpp.BackBufferWidth / 4 + 2, d3dpp.BackBufferWidth / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexBrightPass, NULL );
	/*
	g_pd3dDevice->CreateTexture( g_D3dpp.BackBufferWidth / 4 + 2, g_D3dpp.BackBufferWidth / 4 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexStarSource, NULL );*/

	g_pd3dDevice->CreateTexture( d3dpp.BackBufferWidth / 8 + 2, d3dpp.BackBufferWidth / 8 + 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&g_pTexBloomSource, NULL );

	for(int i = 0; i < NUM_BLOOM_TEXTURES; i++ )
	{
		g_pd3dDevice->CreateTexture( d3dpp.BackBufferWidth / 8 + 2, d3dpp.BackBufferWidth / 8 + 2,
			1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT, &g_apTexBloom[i], NULL );
	}
}
void HDRLight::SetSurfHDR()
{
	g_pd3dDevice->GetRenderTarget(0,&pSurfLDR);
	g_pd3dDevice->GetDepthStencilSurface(&pSurfDS);

	g_pTexScene->GetSurfaceLevel(0,&pSurfHDR);

	g_pd3dDevice->SetRenderTarget(0,pSurfHDR);
}
void HDRLight::Render()
{
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

			DrawFullScreenQuad(g_pd3dDevice, 0.0f, 0.0f, 1.0f, 1.0f );

			g_HDREffect->EndPass();
		}
	}
	g_HDREffect->End();
}
void HDRLight::_scene_to_sceneScaled()
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
	GetTextureCoords( g_pTexScene, &rectSrc, g_pTexSceneScaled, NULL, &coords );
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];
	GetSampleOffsets_DownScale4x4(g_D3dpp.BackBufferWidth,g_D3dpp.BackBufferHeight,avSampleOffsets);
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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV, coords.fRightU,coords.fBottomV);

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	SAFE_RELEASE(pSurfScaledScene);
}
void HDRLight::_measureLuminance()
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
	g_HDREffect->Begin( &uiPassCount, 0 );
	for( uiPass = 0; uiPass < uiPassCount; uiPass++ )
	{
		g_HDREffect->BeginPass( uiPass );

		// Draw a fullscreen quad to sample the RT
		DrawFullScreenQuad( g_pd3dDevice,0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();

	dwCurTexture--;
	while( dwCurTexture > 0 )
	{
		g_apTexToneMap[dwCurTexture + 1]->GetLevelDesc( 0, &desc );
		GetSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );


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
			DrawFullScreenQuad(g_pd3dDevice, 0.0f, 0.0f, 1.0f, 1.0f );

			g_HDREffect->EndPass();
		}

		g_HDREffect->End();
		dwCurTexture--;
	}
	g_apTexToneMap[1]->GetLevelDesc( 0, &desc );
	GetSampleOffsets_DownScale4x4( desc.Width, desc.Height, avSampleOffsets );

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
		DrawFullScreenQuad(g_pd3dDevice, 0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();

	for( i = 0; i < NUM_TONEMAP_TEXTURES; i++ )
	{
		SAFE_RELEASE( apSurfToneMap[i] );
	}
}
void HDRLight::_calculateAdaptation()
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
	g_HDREffect->SetFloat( "g_fElapsedTime", 0.5);

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
		DrawFullScreenQuad(g_pd3dDevice, 0.0f, 0.0f, 1.0f, 1.0f );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();


	SAFE_RELEASE( pSurfAdaptedLum );
}
void HDRLight::_sceneScaled_To_BrightPass()
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
	GetTextureRect( g_pTexSceneScaled, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect( g_pTexBrightPass, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords( g_pTexSceneScaled, &rectSrc, g_pTexBrightPass, &rectDest, &coords );

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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	SAFE_RELEASE( pSurfBrightPass );
}
void HDRLight::_brightSource_ToBloomSource()
{
	D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

	// Get the new render target surface
	PDIRECT3DSURFACE9 pSurfBloomSource;
	g_pTexBloomSource->GetSurfaceLevel( 0, &pSurfBloomSource );

	// Get the rectangle describing the sampled portion of the source texture.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectSrc;
	GetTextureRect( g_pTexBrightPass, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	// Get the destination rectangle.
	// Decrease the rectangle to adjust for the single pixel black border.
	RECT rectDest;
	GetTextureRect( g_pTexBloomSource, &rectDest );
	InflateRect( &rectDest, -1, -1 );

	// Get the correct texture coordinates to apply to the rendered quad in order 
	// to sample from the source rectangle and render into the destination rectangle
	CoordRect coords;
	GetTextureCoords( g_pTexBrightPass, &rectSrc, g_pTexBloomSource, &rectDest, &coords );

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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}

	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	SAFE_RELEASE( pSurfBloomSource );
}
/*
void HDRLight::_starSource_To_BloomSource()
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
void HDRLight::_renderBloom()
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
	GetTextureRect( g_pTexBloomSource, &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	RECT rectDest;
	GetTextureRect( g_apTexBloom[2], &rectDest );
	InflateRect( &rectDest, -1, -1 );

	CoordRect coords;
	GetTextureCoords( g_pTexBloomSource, &rectSrc, g_apTexBloom[2], &rectDest, &coords );

	D3DSURFACE_DESC desc;
	g_pTexBloomSource->GetLevelDesc( 0, &desc );


	g_HDREffect->SetTechnique( "GaussBlur5x5" );

	GetSampleOffsets_GaussBlur5x5( desc.Width, desc.Height, avSampleOffsets, avSampleWeights, 1.0f );

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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );

	g_apTexBloom[2]->GetLevelDesc( 0, &desc );

	GetSampleOffsets_Bloom( desc.Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();
	g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );


	g_apTexBloom[1]->GetLevelDesc( 0, &desc );

	GetSampleOffsets_Bloom( desc.Height, afSampleOffsets, avSampleWeights, 3.0f, 2.0f );
	for( i = 0; i < MAX_SAMPLES; i++ )
	{
		avSampleOffsets[i] = D3DXVECTOR2( 0.0f, afSampleOffsets[i] );
	}


	GetTextureRect( g_apTexBloom[1], &rectSrc );
	InflateRect( &rectSrc, -1, -1 );

	GetTextureCoords( g_apTexBloom[1], &rectSrc, g_apTexBloom[0], NULL, &coords );


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
		DrawFullScreenQuad(g_pd3dDevice, coords.fLeftU,coords.fTopV,coords.fRightU,coords.fBottomV );

		g_HDREffect->EndPass();
	}
	g_HDREffect->End();

	SAFE_RELEASE( pSurfBloomSource );
	SAFE_RELEASE( pSurfTempBloom );
	SAFE_RELEASE( pSurfBloom );
	SAFE_RELEASE( pSurfHDR );
	SAFE_RELEASE( pSurfScaledHDR );
}