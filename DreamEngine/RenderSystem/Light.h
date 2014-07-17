#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "D3DHeader.h"
#include "Camera.h"

class Light
{
public:
	Light();
	virtual ~Light();
	void SetPosition(const D3DXVECTOR3& posVec);
	void SetDirection(const D3DXVECTOR3& dirVec);
	const D3DXVECTOR3& GetPosition() const;
	const D3DXVECTOR3& GetDirection() const;

	void SetColor(const D3DXVECTOR4& color);
	const D3DXVECTOR4& GetColor() const;

	// 返回当前的视矩阵
	const D3DXMATRIX *GetViewTrans() const;
	//返回当前的投影矩阵
	const D3DXMATRIX *GetProjTrans() const;
	void Update();
	float GetCosTheta();
private:
	CLightCamera mCamera;

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mDirection;

	D3DXVECTOR4 mColor;
	float mCosTheta;
};
inline float Light::GetCosTheta()
{
	return mCosTheta;
}
inline const D3DXMATRIX* Light::GetViewTrans() const
{
	return mCamera.GetViewTrans();
}
inline const D3DXMATRIX* Light::GetProjTrans() const
{
	return mCamera.GetProjTrans();
}
inline void Light::SetPosition(const D3DXVECTOR3& posVec)
{
	mPosition = posVec;
}
inline void Light::SetDirection(const D3DXVECTOR3& dirVec)
{
	mDirection = dirVec;
}
inline const D3DXVECTOR3& Light::GetPosition() const
{
	return mCamera.GetEyePos();
}
inline const D3DXVECTOR3& Light::GetDirection() const
{
	return mCamera.GetEyeDir();
}
inline void Light::SetColor(const D3DXVECTOR4& color)
{
	mColor = color;
}
inline const D3DXVECTOR4& Light::GetColor() const
{
	return mColor;
}


#endif