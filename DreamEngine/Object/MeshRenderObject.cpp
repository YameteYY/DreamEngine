#include "MeshRenderObject.h"
#include "../RenderSystem/D3DRender.h"
#include "../RenderSystem/Camera.h"

MeshRenderObject::MeshRenderObject()
{
	mMaterial.clear();
	mDiffuseMap.clear();
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
				sprintf(str,"Media/%s",mtrls[i].pTextureFilename);
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					str,
					&tex);
				mDiffuseMap.push_back( tex );
			}
			else
			{
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					"Media/rocks.jpg",
					&tex);
				mDiffuseMap.push_back( tex );
			}
		}
	}
	SAFE_RELEASE(mtrlBuffer);

	D3DXCreateTextureFromFile(
		Device,
		"Media/rocks_NM_height.tga",
		&mNormalMap);
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

	mEffect->SetTechnique(mTechHandle);
	UINT numPasses = 0;
	mEffect->Begin(&numPasses,0);
	mEffect->SetTexture("NormalMap",mNormalMap);
	mEffect->SetVector("g_LightDir",&D3DXVECTOR4(0,-1,0,0));
	const D3DXVECTOR3& eyePos = camera->GetEyePos();
	mEffect->SetVector("g_EyePos",&D3DXVECTOR4(eyePos.x,eyePos.y,eyePos.z,1.0f) );
	for (int i=0;i<mNumMesh;i++)
	{
		mEffect->BeginPass(i);
		mEffect->SetTexture("DiffuseMap",mDiffuseMap[i]);
		mMesh->DrawSubset(i);
		mEffect->EndPass();
	}
	mEffect->End();
}