#ifndef __MESHRENDEROBJECT_H__
#define __MESHRENDEROBJECT_H__
#include "RenderObject.h"

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 texcoord;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 targent;
	D3DXVECTOR3 binormal;
};
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
	std::vector<IDirect3DTexture9*> mDiffuseMap;
	IDirect3DTexture9* mNormalMap;
	D3DXATTRIBUTERANGE* mMeshTable;
	IDirect3DVertexBuffer9* mVertexBuffer;
	IDirect3DIndexBuffer9*  mIndexBuffer;
	IDirect3DVertexDeclaration9* mVertexDecl;
	int mVertexSize;
};

#endif