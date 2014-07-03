#include "MeshRenderObject.h"
#include "../RenderSystem/D3DRender.h"
#include "../RenderSystem/Camera.h"

MeshRenderObject::MeshRenderObject()
{
	mMaterial.clear();
	mTexture.clear();
}
MeshRenderObject::~MeshRenderObject()
{

}
bool MeshRenderObject::Init(const char* meshName)
{
	HRESULT hr = 0;
	
	//
	// Load the XFile data.  
	//
	LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();
	ID3DXBuffer* mtrlBuffer = 0;
	mNumMesh   = 0;

	hr = D3DXLoadMeshFromX(  
		meshName,
		D3DXMESH_MANAGED,
		Device,
		0,
		&mtrlBuffer,
		0,
		&mNumMesh,
		&mMesh);

	if(FAILED(hr))
	{
		::MessageBox(0, "D3DXLoadMeshFromX() - FAILED", 0, 0);
		return false;
	}

	if( mtrlBuffer != 0 && mNumMesh != 0 )
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();
		for(int i = 0; i < mNumMesh; i++)
		{
			mtrls[i].MatD3D = mtrls[i].MatD3D;
			mMaterial.push_back( mtrls[i].MatD3D );
			if( mtrls[i].pTextureFilename != 0 )
			{
				char str[64];
				sprintf(str,"res/%s",mtrls[i].pTextureFilename);
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					str,
					&tex);
				mTexture.push_back( tex );
			}
			else
			{
				mTexture.push_back( 0 );
			}
		}
	}
	SAFE_RELEASE(mtrlBuffer);
	return true;
}
void MeshRenderObject::Render()
{
	CCamera* camera = D3DRender::Instance()->GetCamera();
	
	mEffect->SetMatrix("g_mWorld",&mWord);
	const D3DXMATRIX *mProj = camera->GetProjTrans();
	const D3DXMATRIX *mView = camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);
	mEffect->SetMatrix("g_mWorldViewProjection",&vp);
	/*
	D3DXVECTOR3 position( cosf(1.0) * 10.0f, 50, sinf(1.0) * 10.0f );
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);

	D3DXMATRIX W, P;

	D3DXMatrixIdentity(&W);
	mEffect->SetMatrix("g_mWorld",&W);

	D3DXMatrixPerspectiveFovLH(
		&P,	D3DX_PI * 0.25f, // 45 - degree
		(float)800 / (float)600,
		1.0f, 1000.0f);

	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,&V,&P);
	mEffect->SetMatrix( "g_mWorldViewProjection", &vp);
	*/
	mEffect->SetTechnique(mTechHandle);
	UINT numPasses = 0;
	mEffect->Begin(&numPasses,0);
	for (int i=0;i<mNumMesh;i++)
	{
		mEffect->BeginPass(i);
		mEffect->SetTexture("Tex",mTexture[i]);
		mMesh->DrawSubset(i);
		mEffect->EndPass();
	}
	mEffect->End();
}