#ifndef __RENDEROBJECT_H__
#define __RENDEROBJECT_H__
#include "../RenderSystem/D3DHeader.h"

class RenderObject
{
public:
	virtual void Render() = 0;
	virtual ~RenderObject();
private:
	ID3DXEffect* mEffect;

};

#endif