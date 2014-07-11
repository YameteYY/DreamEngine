#include "MeshRenderObject.h"
#include "../RenderSystem/D3DRender.h"
#include "../RenderSystem/Camera.h"

MeshRenderObject::MeshRenderObject()
{
	mMaterial.clear();
	mDiffuseMap.clear();
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
		0,
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
		for(int i = 0; i < mNumMesh; i++)
		{
			mtrls[i].MatD3D = mtrls[i].MatD3D;
			mMaterial.push_back( mtrls[i].MatD3D );
			if( mtrls[i].pTextureFilename != 0 )
			{
				char str[64];
				sprintf(str,"Media/%s",mtrls[i].pTextureFilename);
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					str,
					&tex);
				mDiffuseMap.push_back( tex );
			}
			else
			{
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					"Media/wood.jpg",
					&tex);
				mDiffuseMap.push_back( tex );
			}
		}
	}
	SAFE_RELEASE(mtrlBuffer);

	D3DXCreateTextureFromFile(
		Device,
		"Media/four_NM_height.tga",
		&mNormalMap);
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
		return E_FAIL;
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
		return E_FAIL;
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
		return E_OUTOFMEMORY;
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
			D3DDECLUSAGE_NORMAL, 0, 0, rgdwAdjacency, -1.01f,
			-0.01f, -1.01f, &pNewMesh, NULL ) ) )
		{
			return E_FAIL;
		}

		SAFE_RELEASE( mMesh );
		mMesh = pNewMesh;
	}

	SAFE_DELETE_ARRAY( rgdwAdjacency );

	//Optimize the mesh 
	mMesh->Optimize(D3DXMESH_MANAGED | 
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, 
		(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &mMesh); 

	SAFE_RELEASE(adjBuffer); // Done with buffer.

	return true;
}
void MeshRenderObject::Render()
{
	if( GetKeyState('N') & 0x8000 )
	{
		mTechHandle = mEffect->GetTechniqueByName("NMTechnique");
	}
	else if( GetKeyState('P') & 0x8000 )
	{
		mTechHandle = mEffect->GetTechniqueByName("PMTechnique");
	}
	CCamera* camera = D3DRender::Instance()->GetCamera();
	
	mEffect->SetMatrix("g_mWorld",&mWord);
	const D3DXMATRIX *mProj = camera->GetProjTrans();
	const D3DXMATRIX *mView = camera->GetViewTrans();
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,mView,mProj);
	mEffect->SetMatrix("g_mWorldViewProjection",&vp);

	mEffect->SetTechnique(mTechHandle);
	UINT numPasses = 0;
	mEffect->Begin(&numPasses,0);
	mEffect->SetTexture("NormalMap",mNormalMap);
	mEffect->SetVector("g_LightDir",&D3DXVECTOR4(1,-1,0,0));
	const D3DXVECTOR3& eyePos = camera->GetEyePos();
	mEffect->SetVector("g_EyePos",&D3DXVECTOR4(eyePos.x,eyePos.y,eyePos.z,1.0f) );

	for(int j=0;j<numPasses;j++)
	{
		mEffect->BeginPass(j);
		//for (int i=0;i<mNumMesh;i++)
		{
			int i = 0;
			mEffect->SetTexture("DiffuseMap",mDiffuseMap[i]);
			mEffect->SetVector("g_materialAmbientColor",&D3DXVECTOR4(mMaterial[i].Ambient.r,mMaterial[i].Ambient.g,
				mMaterial[i].Ambient.b,mMaterial[i].Ambient.a) );
			mEffect->SetVector("g_materialDiffuseColor",&D3DXVECTOR4(mMaterial[i].Diffuse.r,mMaterial[i].Diffuse.g,
				mMaterial[i].Diffuse.b,mMaterial[i].Diffuse.a));
			mEffect->SetVector("g_materialSpecularColor",&D3DXVECTOR4(mMaterial[i].Specular.r,mMaterial[i].Specular.g,
				mMaterial[i].Specular.b,mMaterial[i].Specular.a));
			mEffect->CommitChanges();
			mMesh->DrawSubset(i);
		}
		mEffect->EndPass();
	}
	mEffect->End();
}