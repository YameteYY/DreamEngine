#ifndef __MATH_H__
#define __MATH_H__
#include "D3DHeader.h"

static void GetTextureCoords( PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc, PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords )
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
//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2
// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
//       pixel shader.
//-----------------------------------------------------------------------------
static HRESULT GetSampleOffsets_DownScale2x2( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
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
static void GetSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 avSampleOffsets[] )
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

static void GetTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect )
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
static float GaussianDistribution( float x, float y, float rho )
{
	float g = 1.0f / sqrtf( 2.0f * D3DX_PI * rho * rho );
	g *= expf( -( x * x + y * y ) / ( 2 * rho * rho ) );

	return g;
}
static void GetSampleOffsets_GaussBlur5x5( DWORD dwD3DTexWidth,
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

static void GetSampleOffsets_Bloom( DWORD dwD3DTexSize,
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
static void DrawFullScreenQuad(LPDIRECT3DDEVICE9  g_pd3dDevice, float fLeftU, float fTopV, float fRightU, float fBottomV )
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

#endif