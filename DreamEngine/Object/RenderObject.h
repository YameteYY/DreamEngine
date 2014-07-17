#ifndef __RENDEROBJECT_H__
#define __RENDEROBJECT_H__
#include "../RenderSystem/D3DHeader.h"
#include "../RenderSystem/Material.h"
#include <vector>

class RenderObject
{
public:
	RenderObject();
	virtual void Render(RenderType type) = 0;
	bool SetEffectFromFile(const char* shaderFile);
	virtual ~RenderObject();
	ID3DXEffect* GetEffect();
	void SetWordTransform(const D3DXMATRIX& word);
	void SetShadowTech();
protected:
	D3DXMATRIX mWord;
	ID3DXEffect* mEffect;
	D3DXHANDLE mSurfaceTechHandle;
	D3DXHANDLE mShadowTechHandle;
};

inline ID3DXEffect* RenderObject::GetEffect()
{
	return mEffect;
}
inline void RenderObject::SetWordTransform(const D3DXMATRIX& word)
{
	mWord = word;
}
inline void RenderObject::SetShadowTech()
{
	mSurfaceTechHandle = mEffect->GetTechniqueByName("ShadowTechnique");
}
#endif