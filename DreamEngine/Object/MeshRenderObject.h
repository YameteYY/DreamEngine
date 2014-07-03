#ifndef __MESHRENDEROBJECT_H__
#define __MESHRENDEROBJECT_H__
#include "RenderObject.h"

class MeshRenderObject : public RenderObject
{
public:
	MeshRenderObject();
	~MeshRenderObject();
	bool Init(const char* meshName);
	virtual void Render();
private:
	ID3DXMesh* mMesh;
	std::vector<D3DMATERIAL9> mMaterial;
	std::vector<IDirect3DTexture9*> mTexture;
};

#endif