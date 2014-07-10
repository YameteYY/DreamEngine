#include "RenderObject.h"
#include "../RenderSystem/D3DRender.h"

RenderObject::RenderObject()
{
	mEffect = NULL;
	mTechHandle = NULL;
	D3DXMatrixIdentity(&mWord);
}
RenderObject::~RenderObject()
{
	SAFE_RELEASE(mEffect);
}
bool RenderObject::SetEffectFromFile(const char* shaderFile)
{
	LPDIRECT3DDEVICE9 g_pDevice = D3DRender::Instance()->GetDevice();
	ID3DXBuffer* errorBuffer = 0;
	HRESULT hr = D3DXCreateEffectFromFile(
		g_pDevice,
		shaderFile,
		0,                // no preprocessor definitions
		0,                // no ID3DXInclude interface
		D3DXSHADER_DEBUG, // compile flags
		0,                // don't share parameters
		&mEffect,
		&errorBuffer);

	// output any error messages
	if( errorBuffer )
	{
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		SAFE_RELEASE(errorBuffer);
	}

	if(FAILED(hr))
	{
		::MessageBox(0, "D3DXCreateEffectFromFile() - FAILED", 0, 0);
		return false;
	}

	mTechHandle = mEffect->GetTechniqueByName("NMTechnique");

	/*
	const D3DVERTEXELEMENT9 decl[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, 
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0}, 
		D3DDECL_END()
	};

	IDirect3DVertexDeclaration9* pDecl;
	g_pDevice->CreateVertexDeclaration(decl,&pDecl);
	g_pDevice->SetVertexDeclaration(pDecl);*/



	return true;
}