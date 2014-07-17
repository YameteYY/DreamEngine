#include "Light.h"

Light::Light():mPosition(5,8,0),mDirection(0,-1,0),mColor(1,1,1,1)
{
	mCamera.SetProjParams(D3DX_PI * 0.25f,(float)4.0f / (float)3.0f,1.0f,1000.0f );
	D3DXVECTOR3 lookat = mPosition + mDirection;
	mCamera.SetViewParams(mPosition, lookat, D3DXVECTOR3(1,0,0));
	mCosTheta = cos(D3DX_PI * 0.25f*0.5f);
}
Light::~Light()
{

}
void Light::Update()
{
	mCamera.Update();
}