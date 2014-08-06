#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__
#include "Light.h"

class SpotLight : public Light
{
public:
	SpotLight();
	virtual ~SpotLight();
	void SetInnerAngle(float innerAngle);
	float GetInnerAngle();
	void SetOuterAngle(float outAngle);
	float GetOuterAngle();

	void SetDistance(float distance);
	float GetDistance();

	float GetCosHalfOutAngle();
	virtual void SetShaderParam(ID3DXEffect* effect);
	virtual void InitCamera();
	virtual void Render(ID3DXEffect* effect);
private:
	virtual void _buildShape();
	float mInnerAngle;
	float mOuterAngle;
	float mDistance;
};

inline void SpotLight::SetInnerAngle(float innerAngle)
{
	mInnerAngle = innerAngle;
}
inline float SpotLight::GetInnerAngle()
{
	return mInnerAngle;
}
inline void SpotLight::SetOuterAngle(float outAngle)
{
	mOuterAngle = outAngle;
}
inline float SpotLight::GetOuterAngle()
{
	return mOuterAngle;
}
inline float SpotLight::GetCosHalfOutAngle()
{
	return cos(mOuterAngle*0.5);
}
inline void SpotLight::SetDistance(float distance)
{
	mDistance = distance;
}
inline float SpotLight::GetDistance()
{
	return mDistance;
}

#endif