#include "Light.h"

Light::Light():mPosition(5,5,0),mDirection(-1,-1,0),mColor(1,1,1,1)
{
	mCamera.SetProjParams(D3DX_PI * 0.25f,(float)4.0f / (float)3.0f,1.0f,1000.0f );
	D3DXVec3Normalize(&mDirection,&mDirection);
	D3DXVECTOR3 lookat = mPosition + mDirection;
	mCamera.SetViewParams(mPosition, lookat, D3DXVECTOR3(1,0,0));
}
Light::~Light()
{

}
void Light::InitCamera()
{
	mCamera.SetProjParams(D3DX_PI * 0.25f,(float)4.0f / (float)3.0f,1.0f,1000.0f );
	D3DXVECTOR3 lookat = mPosition + mDirection;
	mCamera.SetViewParams(mPosition, lookat, D3DXVECTOR3(1,0,0));
}
void Light::Update()
{
	mCamera.Update();
	mDirection = mCamera.GetEyeDir();
	mPosition = mCamera.GetEyePos();
}
void Light::SetShaderParam(ID3DXEffect* effect)
{
	D3DXMATRIX LightVp;
	D3DXMatrixMultiply(&LightVp,mCamera.GetViewTrans(),mCamera.GetProjTrans());
	effect->SetMatrix("g_mLightVP",&LightVp);
	effect->SetVector("g_LightDir",&D3DXVECTOR4(mDirection.x,mDirection.y,mDirection.z,0.0));
	effect->SetVector("g_vLightPos",&D3DXVECTOR4(mPosition.x,mPosition.y,mPosition.z,0.0));
	effect->SetVector("g_LightColor",&mColor);
}