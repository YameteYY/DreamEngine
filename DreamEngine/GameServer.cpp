#include "GameServer.h"
#include "RenderSystem/D3DRender.h"
#include "Object/MeshRenderObject.h"
#include "RenderSystem/SpotLight.h"
#include "RenderSystem/Material.h"
#include "RenderSystem/TextureMgr.h"

bool GameServer::Init(HWND hWnd)
{
	mD3DRender = D3DRender::Instance();
	mD3DRender->InitD3D(hWnd);

	// Setup the camera's view parameters
	D3DXVECTOR3 vecUp ( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 vecEye( 5.0f, 5.0f, 5.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
	g_camera.SetViewParams(vecEye,vecAt,vecUp);
	mD3DRender->SetCamera(&g_camera);

	g_camera.SetProjParams(D3DX_PI * 0.25f,(float)4.0f / (float)3.0f,1.0f,1000.0f);

	mMesh = new MeshRenderObject();
	mMesh->SetEffectFromFile("res/mesh.fx");
	mMesh->Init("Media/Disc.x");
	mMesh->SetParallaxMapRender();

	std::vector<NormapMaterial>& mat = mMesh->GetMaterial();
	for(int i=0;i<mat.size();i++)
	{
		mat[i].NormalMap = TextureMgr::Instance()->GetTexture("Media/four_NM_height.tga");
	}

	D3DXMATRIX word,scale,rotation;
	D3DXMatrixScaling(&scale,0.05,0.05,0.05);
	D3DXMatrixRotationZ(&rotation,90);
	D3DXMatrixMultiply(&word,&rotation,&scale);
	mMesh->SetWordTransform(word);
	mD3DRender->AddRenderObject(mMesh);

	
	mMesh = new MeshRenderObject();
	mMesh->SetEffectFromFile("res/mesh.fx");
	mMesh->Init("Media/room.x");
	mMesh->SetNormalMapRender();
	std::vector<NormapMaterial>& mat1 = mMesh->GetMaterial();
	for(int i=0;i<mat1.size();i++)
	{
		mat1[i].NormalMap = TextureMgr::Instance()->GetTexture("Media/stones_NM_height.tga");
		mat1[i].Specular = 100;
	}

	D3DXMatrixScaling(&scale,5,5,5);
	mMesh->SetWordTransform(scale);

	mD3DRender->AddRenderObject(mMesh);


	SpotLight* light = new SpotLight();
	light->SetInnerAngle(D3DX_PI*0.05f);
	light->SetOuterAngle(D3DX_PI * 0.5f);
	light->InitCamera();
	mD3DRender->AddLight(light);
	//mD3DRender->SetCamera(light->GetLightCamera());
	return true;
}
void GameServer::Close()
{
	delete mD3DRender;
}
void GameServer::Run()
{
	g_camera.Update();
	std::vector<Light*>* lightList = mD3DRender->GetLightList();
	if( GetKeyState('U') & 0x8000 )
	{
		mD3DRender->SetCamera((*lightList)[0]->GetLightCamera());
	}
	if( GetKeyState('M') & 0x8000 )
	{
		mD3DRender->SetCamera(&g_camera);
	}
	for(int i=0;i<lightList->size();i++)
	{
		(*lightList)[i]->Update();
	}
	mD3DRender->Render();
}
