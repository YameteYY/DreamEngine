#include <windows.h>

//--------------------------------------------------------------------------------------
// Name: CleanUp
// Desc: 释放资源
//--------------------------------------------------------------------------------------
void CleanUp()
{

}

//--------------------------------------------------------------------------------------
// Name: WindowProc
// Desc: 窗口过程函数
//--------------------------------------------------------------------------------------
LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{   switch( msg )
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
	wc.style         =  CS_HREDRAW | CS_VREDRAW;
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
    hWnd = CreateWindowEx(	WS_EX_TOPMOST,
		                    "create window",              
							"create window title" ,        
							WS_POPUP,                      
							0,                             
							0,                             
							GetSystemMetrics(SM_CXSCREEN), 
							GetSystemMetrics(SM_CYSCREEN), 
							NULL,                          
							NULL,                          
                            hInstance,                     
							NULL);                        

   if(!hWnd)  
   {
	   return  FALSE;
   }

   
   g_Init = new CD3DInit();

   if(g_Init->GameInit(hWnd))
   {
	   ShowWindow( hWnd, SW_SHOWDEFAULT );
       UpdateWindow( hWnd );
   }
 
  
   while(msg.message != WM_QUIT)
   {
	   fMessage = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
	   if(fMessage)
       {
		   TranslateMessage(&msg);
           DispatchMessage(&msg);
       }
	   else
       {
		   g_Init->Render();

	   }

    }


    CleanUp();
    UnregisterClass("create window", wc.hInstance);

    return 1;
}
