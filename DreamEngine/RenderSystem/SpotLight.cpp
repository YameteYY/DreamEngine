#include "SpotLight.h"

SpotLight::SpotLight()
{
	mInnerAngle = 0;
	mOuterAngle = D3DX_PI * 0.25f;
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
	mCamera.SetProjParams(mOuterAngle,(float)4.0f / (float)3.0f,1.0f,1000.0f );
	D3DXVECTOR3 lookat = mPosition + mDirection;
	mCamera.SetViewParams(mPosition, lookat, D3DXVECTOR3(1,0,0));
}
