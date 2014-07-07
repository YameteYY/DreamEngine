#ifndef __GAMESERVER_H__
#define __GAMESERVER_H__
#include "RenderSystem/Camera.h"
#include <windows.h>

class D3DRender;
class MeshRenderObject;
class GameServer
{
public:
	bool Init(HWND    hWnd);

	void Close();

	void Run();
private:
	MeshRenderObject* mMesh;
	D3DRender* mD3DRender;
    CRPGCamera g_camera;
};


#endif