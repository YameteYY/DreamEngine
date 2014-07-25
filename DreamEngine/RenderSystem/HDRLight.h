#ifndef __HDR_LIGHT_H__
#define __HDR_LIGHT_H__
#include "D3DHeader.h"

#define NUM_TONEMAP_TEXTURES  4       // Number of stages in the 4x4 down-scaling 
#define NUM_BLOOM_TEXTURES    3       // Number of textures used for the bloom

class HDRLight
{
public:
	void Init(LPDIRECT3DDEVICE9 Device,D3DPRESENT_PARAMETERS d3dpp);
	void SetSurfHDR();
	void Render();
protected:
	void _scene_to_sceneScaled();
	void _measureLuminance();
	void _calculateAdaptation();
	void _sceneScaled_To_BrightPass();
	void _brightSource_ToBloomSource();
	//void _starSource_To_BloomSource();
	void _renderBloom();
	D3DPRESENT_PARAMETERS         g_D3dpp;
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
	LPDIRECT3DDEVICE9             g_pd3dDevice  ;  // Direct3D…Ë±∏÷∏’Î
	PDIRECT3DSURFACE9			  pSurfLDR; // Low dynamic range surface for final output
	PDIRECT3DSURFACE9			  pSurfDS;  // Low dynamic range depth stencil surface
	PDIRECT3DSURFACE9			  pSurfHDR; // High dynamic range surface to store 
};


#endif