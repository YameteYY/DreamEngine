#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "D3DHeader.h"
#include "Camera.h"

struct VertexType
{
	float _x,_y,_z,_w;
	VertexType(){}
	VertexType(float x,float y,float z)
	{
		_x = x;_y = y;_z = z;_w = 1;
	}
	static const DWORD FVF;
};
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

	// ���ص�ǰ���Ӿ���
	const D3DXMATRIX *GetViewTrans() const;
	//���ص�ǰ��ͶӰ����
	const D3DXMATRIX *GetProjTrans() const;
	void Update();
	virtual void InitCamera();
	virtual void SetShaderParam(ID3DXEffect* effect);
	CLightCamera* GetLightCamera();
	virtual void Render(ID3DXEffect* effect){}
protected:
	virtual void _buildShape(){}
	CLightCamera mCamera;

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mDirection;
	D3DXMATRIX  mWord;

	D3DXVECTOR4 mColor;
	IDirect3DVertexBuffer9* mVertexBuffer;
	IDirect3DIndexBuffer9*  mIndexBuffer;
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