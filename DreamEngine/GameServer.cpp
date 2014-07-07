#include "GameServer.h"
#include "RenderSystem/D3DRender.h"
#include "Object/MeshRenderObject.h"
#include "RenderSystem/Light.h"

bool GameServer::Init(HWND hWnd)
{
	mD3DRender = D3DRender::Instance();
	mD3DRender->InitD3D(hWnd);

	// Setup the camera's view parameters
	D3DXVECTOR3 vecUp ( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 vecEye( 0.0f, 15.0f, 0.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
	g_camera.SetViewParams(vecEye,vecAt,vecUp);
	mD3DRender->SetCamera(&g_camera);

	g_camera.SetProjParams(D3DX_PI * 0.25f,(float)4.0f / (float)3.0f,1.0f,1000.0f);

	mMesh = new MeshRenderObject();
	mMesh->SetEffectFromFile("res/mesh.fx");
	mMesh->Init("Media/Disc.x");

	D3DXMATRIX word,scale,rotation;
	D3DXMatrixScaling(&scale,0.1,0.1,0.1);
	D3DXMatrixRotationZ(&rotation,90);
	D3DXMatrixMultiply(&word,&rotation,&scale);
	mMesh->SetWordTransform(word);

	mD3DRender->AddRenderObject(mMesh);


	Light* light = new Light();
	light->SetDirection(D3DXVECTOR4(1.0f,-1.0,1.0f,0.0f));
	return true;
}
void GameServer::Close()
{
	delete mD3DRender;
}
void GameServer::Run()
{
	g_camera.Update();
	mD3DRender->Render();
}
