#ifndef __MESHRENDEROBJECT_H__
#define __MESHRENDEROBJECT_H__
#include "RenderObject.h"

class MeshRenderObject : public RenderObject
{
public:
	virtual void Render();
private:
	D3DXHANDLE mWorldMatrixHandle;
	ID3DXMesh* mMesh;
	D3DXHANDLE mTexHandle;
};

#endif