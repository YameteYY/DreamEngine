#include "RenderObject.h"

RenderObject::~RenderObject()
{
	SAFE_RELEASE(mEffect);
}