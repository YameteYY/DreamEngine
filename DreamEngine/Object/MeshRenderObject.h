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
	virtual void Render(RenderType renderType);
	void SetNormalMapRender();
	void SetParallaxMapRender();
	std::vector<NormapMaterial>& GetMaterial();
private:
	DWORD mNumMesh;
	ID3DXMesh* mMesh;
	std::vector<NormapMaterial> mMaterial;

	D3DXATTRIBUTERANGE* mMeshTable;
	IDirect3DVertexBuffer9* mVertexBuffer;
	IDirect3DIndexBuffer9*  mIndexBuffer;
	IDirect3DVertexDeclaration9* mVertexDecl;
	int mVertexSize;
};
inline std::vector<NormapMaterial>& MeshRenderObject::GetMaterial()
{
	return mMaterial;
}
inline void MeshRenderObject::SetNormalMapRender()
{
	mSurfaceTechHandle = mEffect->GetTechniqueByName("NMTechnique");
}
inline void MeshRenderObject::SetParallaxMapRender()
{
	mSurfaceTechHandle = mEffect->GetTechniqueByName("PMTechnique");
}


#endif