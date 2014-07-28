#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "D3DHeader.h"
#include "Camera.h"

class Light
{
public:
	Light();
	virtual ~Light();
	const D3DXVECTOR3& GetPosition() const;
	const D3DXVECTOR3& GetDirection() const;
	
	void SetPosition(const D3DXVECTOR3& pos);
	void SetDirection(const D3DXVECTOR3& dir);

	void SetColor(const D3DXVECTOR4& color);
	const D3DXVECTOR4& GetColor() const;

	// 返回当前的视矩阵
	const D3DXMATRIX *GetViewTrans() const;
	//返回当前的投影矩阵
	const D3DXMATRIX *GetProjTrans() const;
	void Update();
	virtual void InitCamera();
	virtual void SetShaderParam(ID3DXEffect* effect);
	CLightCamera* GetLightCamera();
protected:
	CLightCamera mCamera;

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mDirection;

	D3DXVECTOR4 mColor;
};
inline CLightCamera* Light::GetLightCamera()
{
	return &mCamera;
}
inline const D3DXMATRIX* Light::GetViewTrans() const
{
	return mCamera.GetViewTrans();
}
inline const D3DXMATRIX* Light::GetProjTrans() const
{
	return mCamera.GetProjTrans();
}
inline const D3DXVECTOR3& Light::GetPosition() const
{
	return mPosition;
}
inline const D3DXVECTOR3& Light::GetDirection() const
{
	return mDirection;
}
inline void Light::SetColor(const D3DXVECTOR4& color)
{
	mColor = color;
}
inline const D3DXVECTOR4& Light::GetColor() const
{
	return mColor;
}
inline void Light::SetPosition(const D3DXVECTOR3& pos)
{
	mPosition = pos;
}
inline void Light::SetDirection(const D3DXVECTOR3& dir)
{
	mDirection = dir;
}


#endif