#include "SpotLight.h"
#include "D3DRender.h"

SpotLight::SpotLight()
{
	mInnerAngle = 0;
	mOuterAngle = D3DX_PI * 0.25f;
	mDistance = 100;
}

SpotLight::~SpotLight()
{

}
void SpotLight::SetShaderParam(ID3DXEffect* effect)
{
	Light::SetShaderParam(effect);

	effect->SetFloat("g_fOuterCosTheta", cos(mOuterAngle*0.5));
	effect->SetFloat("g_fInnerCosTheta", cos(mInnerAngle*0.5));
}
void SpotLight::InitCamera()
{
	mCamera.SetProjParams(mOuterAngle,1.0f,1.0f,100.0f );
	D3DXVECTOR3 lookat = mPosition + mDirection;
	mCamera.SetViewParams(mPosition, lookat, D3DXVECTOR3(0,1,0));
	
	D3DXMATRIX trans;
	D3DXMatrixTranslation(&trans,mPosition.x,mPosition.y,mPosition.z);
	D3DXMatrixMultiply(&mWord,&(mCamera.GetRotationMat()),&trans);
	_buildShape();
}
void SpotLight::_buildShape()
{
	LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();
	Device->CreateVertexBuffer(38*sizeof(VertexType),D3DUSAGE_WRITEONLY,VertexType::FVF,D3DPOOL_MANAGED,&mVertexBuffer,0);

	Device->CreateIndexBuffer(216*sizeof(WORD),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&mIndexBuffer,0);

	VertexType* vertices;
	mVertexBuffer->Lock(0,0,(void**)&vertices,0);
	vertices[0] = VertexType(0,0,0);
	float sectionSlice(D3DX_PI*2.0/36.0);
	float rad = mDistance* tan(0.5*mOuterAngle);
	for(int i =1;i<=36;i++)
	{
		vertices[i] = VertexType( rad*cosf(i*sectionSlice),rad*sinf(i*sectionSlice),mDistance);
	}
	vertices[37] = VertexType( 0,0,mDistance);
	mVertexBuffer->Unlock();

	WORD* indices = 0;
	mIndexBuffer->Lock(0,0,(void**)&indices,0);
	for(int i = 0;i<35;i++)
	{
		indices[i*3] = 0;
		indices[i*3+1] = i + 1;
		indices[i*3+2] = i + 2;
	}
	indices[105] = 0;indices[106] = 36;indices[107] = 1;

	for(int i = 0;i<35;i++)
	{
		indices[108+i*3] = 37;
		indices[108+i*3+1] = i + 2;
		indices[108+i*3+2] = i + 1;
	}
	indices[213] = 37;indices[214] = 1;indices[215] = 36;
	mIndexBuffer->Unlock();
}
void SpotLight::Render(ID3DXEffect* effect)
{
	LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();

	
	Device->SetStreamSource(0,mVertexBuffer,0,sizeof(VertexType));
	Device->SetIndices(mIndexBuffer);
	Device->SetFVF(VertexType::FVF);
	D3DXMATRIX word;
	D3DXMatrixIdentity(&word);
	effect->SetMatrix("g_mWorld",&mWord);
	UINT numPasses = 0;
	effect->Begin(&numPasses,0);
	for(UINT j=0;j<numPasses;j++)
	{
		effect->BeginPass(j);

		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
				0,38,
				0,72);
		effect->EndPass();
	}
	effect->End();
}