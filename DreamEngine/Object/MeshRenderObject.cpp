#include "MeshRenderObject.h"
#include "../RenderSystem/D3DRender.h"
#include "../RenderSystem/Camera.h"
#include "../RenderSystem/TextureMgr.h"

MeshRenderObject::MeshRenderObject():mMeshTable(NULL),mVertexBuffer(NULL),mIndexBuffer(NULL)
{
	mMaterial.clear();
}
MeshRenderObject::~MeshRenderObject()
{

}
bool MeshRenderObject::Init(const char* meshName)
{
	HRESULT hr = 0;
	
	//
	// Load the XFile data.  
	//
	LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();
	ID3DXBuffer* mtrlBuffer = 0;
	ID3DXBuffer* adjBuffer = 0;
	mNumMesh   = 0;

	hr = D3DXLoadMeshFromX(  
		meshName,
		D3DXMESH_MANAGED,
		Device,
		&adjBuffer,
		&mtrlBuffer,
		NULL,
		&mNumMesh,
		&mMesh);

	if(FAILED(hr))
	{
		::MessageBox(0, "D3DXLoadMeshFromX() - FAILED", 0, 0);
		return false;
	}

	if( mtrlBuffer != 0 && mNumMesh != 0 )
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();
		for(DWORD i = 0; i < mNumMesh; i++)
		{
			NormapMaterial mat;
			mat.AmbientColor = D3DXVECTOR4(mtrls[i].MatD3D.Ambient.r,mtrls[i].MatD3D.Ambient.g,
				mtrls[i].MatD3D.Ambient.b,mtrls[i].MatD3D.Ambient.a);

			mat.DiffuseColor = D3DXVECTOR4(mtrls[i].MatD3D.Diffuse.r,mtrls[i].MatD3D.Diffuse.g,
				mtrls[i].MatD3D.Diffuse.b,mtrls[i].MatD3D.Diffuse.a);

			mat.SpecularColor = D3DXVECTOR4(mtrls[i].MatD3D.Specular.r,mtrls[i].MatD3D.Specular.g,
				mtrls[i].MatD3D.Specular.b,mtrls[i].MatD3D.Specular.a);

			if( mtrls[i].pTextureFilename != 0 )
			{
				char str[64];
				sprintf(str,"Media/%s",mtrls[i].pTextureFilename);
				mat.DiffuseMap = TextureMgr::Instance()->GetTexture(str);
			}
			else
			{
				mat.DiffuseMap = TextureMgr::Instance()->GetTexture("Media/wood.jpg");
			}

			mat.NormalMap = TextureMgr::Instance()->GetTexture("Media/Normal.dds");

			mMaterial.push_back( mat );
		}
	}
	SAFE_RELEASE(mtrlBuffer);


	// Create a new vertex declaration to hold all the required data
	const D3DVERTEXELEMENT9 vertexDecl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
		{ 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
		D3DDECL_END()
	};

	LPD3DXMESH pTempMesh = NULL;
	
	// Clone mesh to match the specified declaration: 
	if( FAILED( mMesh->CloneMesh( mMesh->GetOptions(), vertexDecl, Device, &pTempMesh ) ) )
	{
		SAFE_RELEASE( pTempMesh );
		return false;
	}

	//====================================================================//
	// Check if the old declaration contains normals, tangents, binormals //
	//====================================================================//
	bool bHadNormal = false;
	bool bHadTangent = false;
	bool bHadBinormal = false;

	D3DVERTEXELEMENT9 vertexOldDecl[ MAX_FVF_DECL_SIZE ];

	if( mMesh && SUCCEEDED( mMesh->GetDeclaration( vertexOldDecl ) ) )
	{
		// Go through the declaration and look for the right channels, hoping for a match:
		for( UINT iChannelIndex = 0; iChannelIndex < D3DXGetDeclLength( vertexOldDecl ); iChannelIndex++ )
		{
			if( vertexOldDecl[iChannelIndex].Usage == D3DDECLUSAGE_NORMAL )
			{
				bHadNormal = true;
			}

			if( vertexOldDecl[iChannelIndex].Usage == D3DDECLUSAGE_TANGENT )
			{
				bHadTangent = true;
			}

			if( vertexOldDecl[iChannelIndex].Usage == D3DDECLUSAGE_BINORMAL )
			{
				bHadBinormal = true;
			}
		}
	}

	if( pTempMesh == NULL && ( bHadNormal == false || bHadTangent == false || bHadBinormal == false ) )
	{
		// We failed to clone the mesh and we need the tangent space for our effect:
		return false;
	}

	//==============================================================//
	// Generate normals / tangents / binormals if they were missing //
	//==============================================================//
	SAFE_RELEASE( mMesh );
	mMesh = pTempMesh;

	if( !bHadNormal )
	{
		// Compute normals in case the meshes have them
		D3DXComputeNormals( mMesh, NULL );
	}

	DWORD* rgdwAdjacency = NULL;
	rgdwAdjacency = new DWORD[ mMesh->GetNumFaces() * 3 ];

	if( rgdwAdjacency == NULL )
	{
		return false;
	}
	mMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency );

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     
	mMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL );

	if( !bHadTangent || !bHadBinormal )
	{
		ID3DXMesh* pNewMesh;
		
		// Compute tangents, which are required for normal mapping
		if( FAILED( D3DXComputeTangentFrameEx( mMesh, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLUSAGE_TANGENT, 0,
			D3DDECLUSAGE_BINORMAL, 0,
			D3DDECLUSAGE_NORMAL, 0, D3DXTANGENT_GENERATE_IN_PLACE, rgdwAdjacency, -1.01f,
			-0.01f, -1.01f, &pNewMesh, NULL ) ) )
		{
			return false;
		}

		//SAFE_RELEASE( mMesh );
		//mMesh = pNewMesh;
	}
	SAFE_DELETE_ARRAY( rgdwAdjacency );

	//Optimize the mesh 
	mMesh->Optimize(D3DXMESH_MANAGED | 
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, 
		(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &mMesh); 

	SAFE_RELEASE(adjBuffer); // Done with buffer.

	
	mMeshTable = new D3DXATTRIBUTERANGE[mNumMesh];
	mMesh->GetAttributeTable(mMeshTable,NULL);
	mMesh->GetVertexBuffer(&mVertexBuffer);
	mMesh->GetIndexBuffer(&mIndexBuffer);
	Device->CreateVertexDeclaration(vertexDecl,&mVertexDecl);
	
	mVertexSize = sizeof(CUSTOMVERTEX);

	return S_OK;
}
void MeshRenderObject::Render(RenderType renderType)
{
	std::vector<Light*>* light = D3DRender::Instance()->GetLightList();
	LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();
	if(renderType == Surface)
	{
		if(mSurfaceTechHandle == NULL)
			return;
		mEffect->SetTechnique(mSurfaceTechHandle);
	}
	else if(renderType == Shadow)
	{
		if(mShadowTechHandle == NULL)
			return;
		mEffect->SetTechnique(mShadowTechHandle);
	}
	else if(renderType == GBuffer)
	{
		mEffect->SetTechnique("GBufferTechnique");
	}
	UINT numPasses = 0;
	mEffect->SetMatrix("g_mWorld",&mWord);

	Device->SetVertexDeclaration(mVertexDecl);
	Device->SetStreamSource(0,mVertexBuffer,0,mVertexSize);
	Device->SetIndices(mIndexBuffer);

	mEffect->Begin(&numPasses,0);
	for(UINT j=0;j<numPasses;j++)
	{
		mEffect->BeginPass(j);
		for (DWORD i=0;i<mNumMesh;i++)
		{
			mMaterial[i].SetParam(mEffect);

			mEffect->CommitChanges();
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,
				mMeshTable[i].VertexStart,mMeshTable[i].VertexCount,
				mMeshTable[i].FaceStart*3,mMeshTable[i].FaceCount);
		}
		mEffect->EndPass();
	}
	mEffect->End();
}