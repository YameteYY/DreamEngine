#ifndef __MATERIAL_H__
#define __MATERIAL_H__
#include "D3DHeader.h"

struct Material
{
	D3DXVECTOR4 AmbientColor;
	D3DXVECTOR4 DiffuseColor;
	D3DXVECTOR4 SpecularColor;
	IDirect3DTexture9* DiffuseMap;
	Material():AmbientColor(0,0,0,0),DiffuseColor(1,1,1,1),SpecularColor(1,1,1,1),DiffuseMap(NULL)
	{
	}
	virtual void SetParam(ID3DXEffect* effect)
	{
		effect->SetVector("g_materialAmbientColor",&AmbientColor);
		effect->SetVector("g_materialDiffuseColor",&DiffuseColor);
		effect->SetVector("g_materialSpecularColor",&SpecularColor);
		if(DiffuseMap != NULL)
			effect->SetTexture("DiffuseMap",DiffuseMap);
	}
};

struct NormapMaterial : public Material
{
	IDirect3DTexture9* NormalMap;
	float Specular;
	NormapMaterial():NormalMap(NULL),Specular(5)
	{
	}
	virtual void SetParam(ID3DXEffect* effect)
	{
		Material::SetParam(effect);
		if(NormalMap != NULL)
			effect->SetTexture("NormalMap",NormalMap);
		effect->SetFloat("g_specular",Specular);
	}
};

struct ParallaxMaterial : public NormapMaterial
{
	float HeightScale;
	ParallaxMaterial():HeightScale(0.1)
	{
	}
	virtual void SetParam(ID3DXEffect* effect)
	{
		NormapMaterial::SetParam(effect);
		effect->SetFloat("g_heightScale",HeightScale);
	}
};

#endif