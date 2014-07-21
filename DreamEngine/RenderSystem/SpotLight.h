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

	float GetCosHalfOutAngle();
	virtual void SetShaderParam(ID3DXEffect* effect);
	void InitCamera();
private:
	float mInnerAngle;
	float mOuterAngle;
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

#endif