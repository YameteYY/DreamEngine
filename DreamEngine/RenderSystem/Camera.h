#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "D3DHeader.h"

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

	//���������Ӿ�������Ĳ���
	void SetViewParams( D3DXVECTOR3 &pos, D3DXVECTOR3 &lookat, D3DXVECTOR3 &up );
	//��������͸��ͶӰ��������Ĳ���
	void SetProjParams( float fFOV, float fAspect, float fNear, float fFar  );
	//����������ƽ��ͶӰ��������Ĳ���
	void SetOrthoProjParams( float w, float h, float fNear, float fFar );
	// ���ص�ǰ���Ӿ���
	const D3DXMATRIX *GetViewTrans() const;
	//���ص�ǰ��ͶӰ����
	const D3DXMATRIX *GetProjTrans() const;

	const D3DXVECTOR3& GetEyePos() const;
	const D3DXVECTOR3& GetEyeDir() const;
	const D3DXMATRIX& GetRotationMat() const;
	//����������ƶ��ٶ�
	void SetMoveVelocity( float fVelocity );

	//����Ϣѭ���ﴦ����Ϣ
	virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//���߼�֡���洦����Ϣ
	virtual void ProcessKey(float fElapsedTime);

	//�������������
	//ͨ�����߼�֡������ô˺����������ڲ����Զ��ж���Ҫ������Щ���ԡ�
	virtual void Update();
	//���Ӿ����ͶӰ�������ø�D3D�豸
	void ApplyDevice( LPDIRECT3DDEVICE9  pDevice );

public:
	/// �Ӿ���
	D3DXMATRIX m_ViewTrans;
	/// ͶӰ����
	D3DXMATRIX m_ProjTrans;

	/// �����λ��
	D3DXVECTOR3 m_EyePos;
	/// ������Ĺ۲��
	D3DXVECTOR3 m_LookAt;
	/// �������UP����
	D3DXVECTOR3 m_Up;
	/// �������RIGHT����
	D3DXVECTOR3 m_Right;
	/// ������ķ���
	D3DXVECTOR3 m_Direction;

	D3DXMATRIX  m_Rotation;
	/// ���ӱ���ľ���
	float m_fNear;
	/// Զ�ӱ���ľ���
	float m_fFar;
	/// �ӽǣ�����
	float m_fFOV;
	/// �����
	float m_fAspect;

	/// ��һ�����λ��
	POINT m_LastPoint;
	/// �û��Ƿ���ת�������
	bool m_bIsRot;
	/// �û��Ƿ�ƽ���������
	bool m_bIsTrans;

	/// �������ʼ״̬��ŷ����Yaw����SetViewParamsʱ��ʼ��
	float m_fCameraYawAngle;
	/// �������ʼ״̬��ŷ����Pitch����SetViewParamsʱ��ʼ��
	float m_fCameraPitchAngle;

	/// �������ƽ����
	D3DXVECTOR3 m_vDelta;
	/// ��������ƶ��ٶȣ���λ/��
	float m_fVelocity;

	/// ��������������ֵ
	float m_fMaxPitch;
	/// �������������Сֵ
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
// ��һ�˳�������࣬�ṩ��һ�˳Ʒ�ʽ��������ƶ�����ת
//�������������֪ʶ�㣺

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(void);
	~CFirstPersonCamera(void);

	//�������������
	//ͨ�����߼�֡������ô˺����������ڲ����Զ��ж���Ҫ������Щ���ԡ�
	void Update(void);
};

class CLightCamera : public CCamera
{
public:
	CLightCamera(void);
	~CLightCamera(void);

	//�������������
	//ͨ�����߼�֡������ô˺����������ڲ����Զ��ж���Ҫ������Щ���ԡ�
	void Update(void);
};

//RPGģʽ������࣬�ṩ�����˳Ʒ�ʽ��������ƶ�����ת
//�������������֪ʶ�㣺
class CRPGCamera : public CCamera
{
private:
	/// ������任����
	D3DXVECTOR3 m_vCenter;
	/// ������ӵ���任���ĵľ���
	float m_fDistance;
	/// ������۲������ľ������ֵ
	float m_fMaxDistance;
	/// ������۲������ľ�����Сֵ
	float m_fMinDistance;
	/// ����������ĺ���ƫ��
	float m_fCenterExc;
	/// �ӵ��Ƿ�������
	bool m_bCenter;

public:
	CRPGCamera(void);
	~CRPGCamera(void);

	//������������� 
	///���������������������ת�ƶ����� 
	//vCenter �任����
	//bCenter �ӵ��Ƿ�������
	void SetCenter(const D3DXVECTOR3 &vCenter);
	//�������������
	//ͨ�����߼�֡������ô˺����������ڲ����Զ��ж���Ҫ������Щ���ԡ�
		//void Update(CTerrain *pTerrain);
		// ����Ϣѭ���ﴦ����Ϣ
		virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//���ø��������ֵ 
	void SetMaxPitchAngle(float fPitch) { m_fMaxPitch = fPitch; };
	//���ø�������Сֵ 
	void SetMinPitchAngle(float fPitch) { m_fMinPitch = fPitch; };

	//����������۲������ľ������ֵ

	void SetMaxDistance(float fDistance) { m_fMaxDistance = fDistance; };
	//�õ�������۲������ľ������ֵ
	float GetMaxDistance() { return m_fMaxDistance; };
	//����������۲������ľ�����Сֵ

	void SetMinDistance(float fDistance) { m_fMinDistance = fDistance; };
	//�õ�������۲������ľ�����Сֵ
	float GetMinDistance() { return m_fMinDistance; };
	//������������ӵ����
	void SetDistance(float fDistance) { m_fDistance = fDistance; };
	//����ƫ����
	//fExc ������ӵ���任���ĵĺ���ƫ����
	void SetExcursion(float fExc);
};

#endif