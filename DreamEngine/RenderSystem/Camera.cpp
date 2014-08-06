#include "Camera.h"

CCamera::CCamera() :
m_fNear(0.f),
	m_fFar(0.f),
	m_fFOV(0.f),
	m_fAspect(0.f),
	m_bIsRot(false),
	m_bIsTrans(false),
	m_fCameraYawAngle(0.f),
	m_fCameraPitchAngle(0.f),
	m_vDelta(0.f, 0.f, 0.f),
	m_fVelocity(0.01f),
	m_fMaxPitch(D3DX_PI*0.49f),
	m_fMinPitch(-D3DX_PI*0.49f)
{
	m_EyePos = D3DXVECTOR3(0.f, 0.f, 0.f);
	m_LookAt = D3DXVECTOR3(0.f, 0.f, 1.f);
	m_Up = D3DXVECTOR3(0.f, 1.f, 0.f);

}

CCamera::~CCamera()
{

}


void CCamera::SetViewParams( D3DXVECTOR3 &pos, D3DXVECTOR3 &lookat, D3DXVECTOR3 &up )
{
	m_EyePos = pos;
	m_LookAt = lookat;
	m_Up = up;

	m_Direction = m_LookAt - m_EyePos;
	D3DXVec3Cross( &m_Right, &m_Up, &m_Direction );


	D3DXMatrixLookAtLH(&m_ViewTrans, &m_EyePos, &m_LookAt, &m_Up);

	D3DXMATRIX mInvView;
	D3DXMatrixInverse( &mInvView, NULL, &m_ViewTrans );
	D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &mInvView._31;
	m_fCameraYawAngle   = atan2f( pZBasis->x, pZBasis->z );
	float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
	m_fCameraPitchAngle = -atan2f( pZBasis->y, fLen );

	D3DXMatrixRotationYawPitchRoll(&m_Rotation, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);
}

void CCamera::SetProjParams( float fFOV, float fAspect, float fNear, float fFar  )
{
	m_fFOV = fFOV;
	m_fAspect = fAspect;
	m_fNear     = fNear;
	m_fFar      = fFar;

	D3DXMatrixPerspectiveFovLH( &m_ProjTrans, m_fFOV, m_fAspect, m_fNear, m_fFar );
}
void CCamera::SetOrthoProjParams( float w, float h, float fNear, float fFar )
{
	D3DXMatrixOrthoLH( &m_ProjTrans, w, h, fNear, fFar );
}
const D3DXMATRIX *CCamera::GetViewTrans() const
{
	return &m_ViewTrans;
}
const D3DXMATRIX *CCamera::GetProjTrans() const
{
	return &m_ProjTrans;
}

void CCamera::SetMoveVelocity( float fVelocity )
{
	m_fVelocity = fVelocity;
}


LRESULT CCamera::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//m_vDelta = D3DXVECTOR3(0.f, 0.f, 0.f);
	// setcapture getcursorPos
	switch( msg )
	{
	case WM_LBUTTONDOWN:
		{
			m_bIsRot = true;
			SetCapture(hWnd);
			GetCursorPos(&m_LastPoint);
		}
		break;
	case WM_LBUTTONUP:
		{
			m_bIsRot = false;
			ReleaseCapture();
		}
		break;
	}

	return 0;
}

void CCamera::ProcessKey(float fElapsedTime)
{
	m_vDelta = D3DXVECTOR3(0.f, 0.f, 0.f);
	float fVelocity = m_fVelocity * fElapsedTime;

	if( GetKeyState('W') & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.z += fVelocity;
	}
	if( GetKeyState('S') & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.z -= fVelocity;
	}
	if( GetKeyState('A') & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.x -= fVelocity;
	}
	if( GetKeyState('D') & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.x += fVelocity;
	}
	if( GetKeyState(VK_HOME) & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.y += fVelocity;
	}
	if( GetKeyState(VK_END) & 0x8000 )
	{
		m_bIsTrans = true;
		m_vDelta.y -= fVelocity;
	}
}


void CCamera::Update()
{
	POINT ptCurrentPos = {0, 0};
	POINT ptDeltaPos = {0, 0};

	if(m_bIsRot)
	{
		//计算鼠标偏移
		GetCursorPos(&ptCurrentPos);
		ptDeltaPos.x = ptCurrentPos.x - m_LastPoint.x;
		ptDeltaPos.y = ptCurrentPos.y - m_LastPoint.y;
		m_LastPoint = ptCurrentPos;

		float fYaw = ptDeltaPos.x*0.01f;
		float fPitch = ptDeltaPos.y*0.01f;

		//根据鼠标便宜计算欧拉角
		m_fCameraYawAngle   += fYaw;
		m_fCameraPitchAngle += fPitch;

	}

	// 根据欧拉角Yaw，Pitch计算摄像机的旋转矩阵
	ZeroMemory(&m_Rotation, sizeof(D3DXMATRIX));
	D3DXMatrixRotationYawPitchRoll(&m_Rotation, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);

	// 根据旋转矩阵将摄像机的局部方向向量和上方向向量转为全局向量
	D3DXVECTOR3 vWorldUp, vWorldAhead;
	D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
	D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &m_Rotation );
	D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &m_Rotation );

	if(m_bIsTrans)
	{
		// 将局部偏移量转到全局坐标
		D3DXVECTOR3 vWorldDelta;
		D3DXVec3TransformCoord( &vWorldDelta, &m_vDelta, &m_Rotation );
		// 根据偏移量计算视点位置
		m_EyePos += vWorldDelta;
	}
	// 计算观察点位置 
	m_LookAt = m_EyePos + vWorldAhead;

	// 更新视矩阵
	D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &vWorldUp );

	m_Direction = m_LookAt - m_EyePos;
	m_Up = vWorldUp;
	D3DXVec3Cross( &m_Right, &vWorldUp, &m_Direction );
}


void CCamera::ApplyDevice( LPDIRECT3DDEVICE9  pDevice )
{
	if( pDevice )
	{
		pDevice->SetTransform(D3DTS_VIEW, &m_ViewTrans);
		pDevice->SetTransform(D3DTS_PROJECTION, &m_ProjTrans);
	}
}

CFirstPersonCamera::CFirstPersonCamera(void)
{
	m_LastPoint.x = 0;
	m_LastPoint.y = 0;
}

CFirstPersonCamera::~CFirstPersonCamera(void)
{
}

void CFirstPersonCamera::Update(void)
{
	if((GetKeyState('L') & 0x8000) )
		return;
	ProcessKey(20);

	POINT ptCurrentPos = {0, 0};
	POINT ptDeltaPos = {0, 0};

	// 计算鼠标偏移
	GetCursorPos(&ptCurrentPos);
	ptDeltaPos.x = ptCurrentPos.x - m_LastPoint.x;
	ptDeltaPos.y = ptCurrentPos.y - m_LastPoint.y;
	m_LastPoint = ptCurrentPos;

	float fYaw = ptDeltaPos.x*0.01f;
	float fPitch = ptDeltaPos.y*0.01f;

	// 根据鼠标偏移计算欧拉角
	m_fCameraYawAngle   += fYaw;
	m_fCameraPitchAngle += fPitch;

	// 判断俯仰角是否越界 
	if(m_fCameraPitchAngle < -D3DX_PI*0.5f)
		m_fCameraPitchAngle = -D3DX_PI*0.5f;
	else if(m_fCameraPitchAngle > D3DX_PI*0.5f)
		m_fCameraPitchAngle = D3DX_PI*0.5f;

	//D3DXMATRIX matCameraRot;
	//ZeroMemory(&matCameraRot, sizeof(D3DXMATRIX));
	D3DXMatrixRotationYawPitchRoll(&m_Rotation, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);

	// 根据旋转矩阵将摄像机的局部方向向量和上方向向量转为全局向量
	D3DXVECTOR3 vWorldUp, vWorldAhead;
	D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
	D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &m_Rotation );
	D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &m_Rotation );

	// first trans, than look at , equal to move to final eye coordinate
	if(m_bIsTrans)
	{
		// 将局部偏移量转到全局坐标
		D3DXVECTOR3 vWorldDelta;
		D3DXVec3TransformCoord( &vWorldDelta, &m_vDelta, &m_Rotation );
		// 根据偏移量计算视点位置
		m_EyePos += vWorldDelta;
	}
	// 计算观察点位置
	m_LookAt = m_EyePos + vWorldAhead;

	// 更新视矩阵
	D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &vWorldUp );

	m_Direction = m_LookAt - m_EyePos;
	m_Up = vWorldUp;
	D3DXVec3Cross( &m_Right, &vWorldUp, &m_Direction );
}

CLightCamera::CLightCamera(void)
{
	m_LastPoint.x = 0;
	m_LastPoint.y = 0;
}

CLightCamera::~CLightCamera(void)
{
}

void CLightCamera::Update(void)
{
	if( ! (GetKeyState('L') & 0x8000) )
		return;
	ProcessKey(20);
	POINT ptCurrentPos = {0, 0};
	POINT ptDeltaPos = {0, 0};

	// 计算鼠标偏移
	GetCursorPos(&ptCurrentPos);
	ptDeltaPos.x = ptCurrentPos.x - m_LastPoint.x;
	ptDeltaPos.y = ptCurrentPos.y - m_LastPoint.y;
	m_LastPoint = ptCurrentPos;

	float fYaw = ptDeltaPos.x*0.01f;
	float fPitch = ptDeltaPos.y*0.01f;

	// 根据鼠标偏移计算欧拉角
	m_fCameraYawAngle   += fYaw;
	m_fCameraPitchAngle += fPitch;

	D3DXMATRIX matCameraRot;
	//ZeroMemory(&matCameraRot, sizeof(D3DXMATRIX));
	D3DXMatrixRotationYawPitchRoll(&matCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);

	// 根据旋转矩阵将摄像机的局部方向向量和上方向向量转为全局向量
	D3DXVECTOR3 vWorldUp, vWorldAhead;
	D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
	D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &matCameraRot );
	D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &matCameraRot );

	// first trans, than look at , equal to move to final eye coordinate
	if(m_bIsTrans)
	{
		// 将局部偏移量转到全局坐标
		D3DXVECTOR3 vWorldDelta;
		D3DXVec3TransformCoord( &vWorldDelta, &m_vDelta, &matCameraRot );
		// 根据偏移量计算视点位置
		m_EyePos += vWorldDelta;
	}
	// 计算观察点位置
	m_LookAt = m_EyePos + vWorldAhead;

	// 更新视矩阵
	D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &vWorldUp );

	m_Direction = m_LookAt - m_EyePos;
	m_Up = vWorldUp;
	D3DXVec3Cross( &m_Right, &vWorldUp, &m_Direction );
}

CRPGCamera::CRPGCamera(void) :
m_fDistance(50),
	m_fMaxDistance(1000.0f),
	m_fMinDistance(100.0f),
	m_fCenterExc(0.0f),
	m_vCenter(0.0f, 0.0f, 0.0f),
	m_bCenter(true)
{
}

CRPGCamera::~CRPGCamera(void)
{
}

//void CRPGCamera::Update(CTerrain *pTerrain)
//{
// POINT ptCurrentPos = {0, 0};
// POINT ptDeltaPos = {0, 0};
//
// if(m_bIsRot)
// {
// // 计算鼠标偏移
// GetCursorPos(&ptCurrentPos);
// ptDeltaPos.x = ptCurrentPos.x - m_LastPoint.x;
// ptDeltaPos.y = ptCurrentPos.y - m_LastPoint.y;
// m_LastPoint = ptCurrentPos;
//
// float fYaw = ptDeltaPos.x*0.01f;
// float fPitch = ptDeltaPos.y*0.01f;
//
// // 根据鼠标偏移计算欧拉角
// m_fCameraYawAngle   += fYaw;
// m_fCameraPitchAngle += fPitch;
//
// // 判断俯仰角是否越界 
// if(m_fCameraPitchAngle > m_fMaxPitch)
// m_fCameraPitchAngle = m_fMaxPitch;
// else if(m_fCameraPitchAngle < m_fMinPitch)
// m_fCameraPitchAngle = m_fMinPitch;
// }
//
//
// D3DXMATRIX matCameraRot;
// ZeroMemory(&matCameraRot, sizeof(D3DXMATRIX));
// D3DXMatrixRotationYawPitchRoll(&matCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);
//
// // 根据旋转矩阵将摄像机的局部方向向量和上方向向量转为全局向量
// D3DXVECTOR3 vWorldUp, vWorldAhead;
// D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
// D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
// D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &matCameraRot );
// D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &matCameraRot );
//
// // 计算视点和观察点位置
// if(m_bCenter)
// {
// m_EyePos = m_vCenter - vWorldAhead*m_fDistance;
// m_LookAt = m_vCenter;
// }
// else
// {
// D3DXVECTOR3 vTran;
// D3DXVec3Cross(&vTran, &vWorldUp, &vWorldAhead);
// m_EyePos = m_vCenter - vWorldAhead*m_fDistance + m_fCenterExc*vTran;
// m_LookAt = m_vCenter + m_fCenterExc*vTran;
// }
//
// // 根据地面高度计算摄像机视点y坐标
// float fTerrainHeight = pTerrain->GetHeight(m_EyePos.x, m_EyePos.z);
// if( m_EyePos.y < fTerrainHeight )
// m_EyePos.y = fTerrainHeight + 3.0f;
//
// // 更新视矩阵
// D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &vWorldUp );
//
// m_Direction = m_LookAt - m_EyePos;
// m_Up = vWorldUp;
// D3DXVec3Cross( &m_Right, &vWorldUp, &m_Direction );
//}

void  CRPGCamera::SetCenter(const D3DXVECTOR3 &vCenter) 
{
	m_vCenter = vCenter;
}

LRESULT CRPGCamera::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ProcessKey(10);
	switch( msg )
	{
	case WM_MOUSEWHEEL: //鼠标滚轮
		{
			short delta = HIWORD(wParam);
			m_fDistance += -delta * 0.1f;
			if ( m_fDistance < m_fMinDistance)
				m_fDistance = m_fMinDistance;
			else if( m_fDistance > m_fMaxDistance )
				m_fDistance = m_fMaxDistance;
		}

		break;

	case WM_RBUTTONDOWN:
		{
			m_bIsRot = true;
			SetCapture(hWnd);
			GetCursorPos(&m_LastPoint);
		}
		break;
	case WM_RBUTTONUP:
		{
			m_bIsRot = false;
			ReleaseCapture();
		}
		break;
	}

	//return CCamera::HandleMessage(hWnd, msg, wParam, lParam);
	return 0;
}

void CRPGCamera::SetExcursion(float fExc)
{
	if(fExc != 0.0f)
	{
		m_bCenter = false;
		m_fCenterExc = fExc;
	}
	else
		m_bCenter = true;
}