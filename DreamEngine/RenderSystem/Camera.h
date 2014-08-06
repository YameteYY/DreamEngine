#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "D3DHeader.h"

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

	//设置生成视矩阵所需的参数
	void SetViewParams( D3DXVECTOR3 &pos, D3DXVECTOR3 &lookat, D3DXVECTOR3 &up );
	//设置生成透视投影矩阵所需的参数
	void SetProjParams( float fFOV, float fAspect, float fNear, float fFar  );
	//设置生成视平行投影矩阵所需的参数
	void SetOrthoProjParams( float w, float h, float fNear, float fFar );
	// 返回当前的视矩阵
	const D3DXMATRIX *GetViewTrans() const;
	//返回当前的投影矩阵
	const D3DXMATRIX *GetProjTrans() const;

	const D3DXVECTOR3& GetEyePos() const;
	const D3DXVECTOR3& GetEyeDir() const;
	const D3DXMATRIX& GetRotationMat() const;
	//设置摄像机移动速度
	void SetMoveVelocity( float fVelocity );

	//在消息循环里处理消息
	virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//在逻辑帧里面处理消息
	virtual void ProcessKey(float fElapsedTime);

	//更新摄像机属性
	//通常在逻辑帧里面调用此函数，函数内部会自动判断需要更新哪些属性。
	virtual void Update();
	//将视矩阵和投影矩阵设置给D3D设备
	void ApplyDevice( LPDIRECT3DDEVICE9  pDevice );

public:
	/// 视矩阵
	D3DXMATRIX m_ViewTrans;
	/// 投影矩阵
	D3DXMATRIX m_ProjTrans;

	/// 摄像机位置
	D3DXVECTOR3 m_EyePos;
	/// 摄像机的观察点
	D3DXVECTOR3 m_LookAt;
	/// 摄像机的UP向量
	D3DXVECTOR3 m_Up;
	/// 摄像机的RIGHT向量
	D3DXVECTOR3 m_Right;
	/// 摄像机的方向
	D3DXVECTOR3 m_Direction;

	D3DXMATRIX  m_Rotation;
	/// 近视表面的距离
	float m_fNear;
	/// 远视表面的距离
	float m_fFar;
	/// 视角，弧度
	float m_fFOV;
	/// 长宽比
	float m_fAspect;

	/// 上一个鼠标位置
	POINT m_LastPoint;
	/// 用户是否旋转了摄像机
	bool m_bIsRot;
	/// 用户是否平移了摄像机
	bool m_bIsTrans;

	/// 摄像机初始状态的欧拉角Yaw，在SetViewParams时初始化
	float m_fCameraYawAngle;
	/// 摄像机初始状态的欧拉角Pitch，在SetViewParams时初始化
	float m_fCameraPitchAngle;

	/// 摄像机的平移量
	D3DXVECTOR3 m_vDelta;
	/// 摄像机的移动速度，单位/秒
	float m_fVelocity;

	/// 摄像机俯仰角最大值
	float m_fMaxPitch;
	/// 摄像机俯仰角最小值
	float m_fMinPitch;
};

inline const D3DXVECTOR3& CCamera::GetEyePos() const
{
	return m_EyePos;
}
inline const D3DXVECTOR3& CCamera::GetEyeDir() const
{
	return m_Direction;
}
inline const D3DXMATRIX& CCamera::GetRotationMat() const
{
	return m_Rotation;
}
// 第一人称摄像机类，提供第一人称方式的摄像机移动和旋转
//此类包括了如下知识点：

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(void);
	~CFirstPersonCamera(void);

	//更新摄像机属性
	//通常在逻辑帧里面调用此函数，函数内部会自动判断需要更新哪些属性。
	void Update(void);
};

class CLightCamera : public CCamera
{
public:
	CLightCamera(void);
	~CLightCamera(void);

	//更新摄像机属性
	//通常在逻辑帧里面调用此函数，函数内部会自动判断需要更新哪些属性。
	void Update(void);
};

//RPG模式摄像机类，提供第三人称方式的摄像机移动和旋转
//此类包括了如下知识点：
class CRPGCamera : public CCamera
{
private:
	/// 摄像机变换中心
	D3DXVECTOR3 m_vCenter;
	/// 摄像机视点与变换中心的距离
	float m_fDistance;
	/// 摄像机观察点距中心距离最大值
	float m_fMaxDistance;
	/// 摄像机观察点距中心距离最小值
	float m_fMinDistance;
	/// 摄像机距中心横向偏移
	float m_fCenterExc;
	/// 视点是否在中心
	bool m_bCenter;

public:
	CRPGCamera(void);
	~CRPGCamera(void);

	//设置摄像机中心 
	///摄像机会以中心来进行旋转移动操作 
	//vCenter 变换中心
	//bCenter 视点是否在中心
	void SetCenter(const D3DXVECTOR3 &vCenter);
	//更新摄像机属性
	//通常在逻辑帧里面调用此函数，函数内部会自动判断需要更新哪些属性。
		//void Update(CTerrain *pTerrain);
		// 在消息循环里处理消息
		virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//设置俯仰角最大值 
	void SetMaxPitchAngle(float fPitch) { m_fMaxPitch = fPitch; };
	//设置俯仰角最小值 
	void SetMinPitchAngle(float fPitch) { m_fMinPitch = fPitch; };

	//设置摄像机观察点距中心距离最大值

	void SetMaxDistance(float fDistance) { m_fMaxDistance = fDistance; };
	//得到摄像机观察点距中心距离最大值
	float GetMaxDistance() { return m_fMaxDistance; };
	//设置摄像机观察点距中心距离最小值

	void SetMinDistance(float fDistance) { m_fMinDistance = fDistance; };
	//得到摄像机观察点距中心距离最小值
	float GetMinDistance() { return m_fMinDistance; };
	//设置摄像机与视点距离
	void SetDistance(float fDistance) { m_fDistance = fDistance; };
	//设置偏移量
	//fExc 摄像机视点与变换中心的横向偏移量
	void SetExcursion(float fExc);
};

#endif