#include <windows.h>
#include "GameServer.h"
#include "RenderSystem/D3DRender.h"
#include "RenderSystem/Camera.h"

//--------------------------------------------------------------------------------------
// Name: WindowProc
// Desc: 窗口过程函数
//--------------------------------------------------------------------------------------
CCamera* g_camera = 0;

LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
        case WM_KEYUP: 
			switch (wParam)
		    { 
			    case VK_ESCAPE:
			         PostQuitMessage( 0 );
			    return 0;
				break;
			}
         return 0;
    }
    return DefWindowProc( hWnd, msg, wParam, lParam );
}



//--------------------------------------------------------------------------------------
// Name: WinMain
// Desc: 主函数
//--------------------------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine,int nCmdShow)
{
    
    HWND        hWnd;
	WNDCLASS    wc;
    MSG         msg; 
    BOOL        fMessage;

	//创建和设置窗口类
	wc.style         =  CS_DBLCLKS;
	wc.lpfnWndProc   =  WindowProc;
	wc.cbClsExtra    =  0;
	wc.cbWndExtra    =  0;
	wc.hInstance     =  hInstance;
	wc.hIcon         =  NULL ;//LoadIcon(NULL,IDI_APP)
	wc.hCursor       =  NULL ;//LoadCursor(NULL,IDC_ARROW)
	wc.hbrBackground =  (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  =  NULL;
	wc.lpszClassName =  "create window";

	RegisterClass(&wc);
	hWnd = CreateWindow( "create window", "test", WS_OVERLAPPEDWINDOW,
		0, 0, 800, 600, 0,
		NULL, hInstance, 0 );                   

   if(!hWnd)  
   {
	   return  FALSE;
   }

   
   GameServer* gameServer = new GameServer();

   if(gameServer->Init(hWnd))
   {
	   ShowWindow( hWnd, SW_SHOWDEFAULT );
       UpdateWindow( hWnd );
   }
   g_camera = D3DRender::Instance()->GetCamera();
   PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
   while(msg.message != WM_QUIT)
   {
	   g_camera->HandleMessage(hWnd,msg.message,msg.lParam,msg.wParam);
	   fMessage = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
	   if(fMessage)
       {
		   TranslateMessage(&msg);
           DispatchMessage(&msg);
       }
	   else
       {
		   gameServer->Run();

	   }

    }
    gameServer->Close();

    UnregisterClass("create window", wc.hInstance);

    return 1;
}
