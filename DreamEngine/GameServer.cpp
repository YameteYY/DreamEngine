#include "GameServer.h"
#include "RenderSystem/D3DRender.h"
#include "Object/MeshRenderObject.h"

bool GameServer::Init(HWND hWnd)
{
	mD3DRender = D3DRender::Instance();
	mD3DRender->InitD3D(hWnd);

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 0.0f, 5.0f, 5.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vecUp ( 0.0f, 1.0f, 0.0f );
	g_camera.SetViewParams( vecEye, vecAt,vecUp);
	g_camera.SetProjParams(45.0f,4.0f/3.0f,1.0f,1000.0f);
	mD3DRender->SetCamera(&g_camera);

	mMesh = new MeshRenderObject();
	mMesh->SetEffectFromFile("res/mesh.fx");
	mMesh->Init("res/tiny.x");

	D3DXMATRIX word;
	D3DXMatrixScaling(&word,0.01,0.01,0.01);
	mMesh->SetWordTransform(word);

	mD3DRender->AddRenderObject(mMesh);

	//mMesh = new MeshRenderObject();
//	mMesh->SetEffectFromFile("res/mesh.fx");
//	mMesh->Init("res/mountain.x");


//	mD3DRender->AddRenderObject(mMesh);
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
