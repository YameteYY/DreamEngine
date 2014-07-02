#include "D3DRender.h"

D3DRender::D3DRender()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
}
D3DRender::~D3DRender()
{
	SAFE_RELEASE(g_pD3D);
	SAFE_RELEASE(g_pd3dDevice);
}
HRESULT  D3DRender::InitD3D( HWND hWnd)
{
	if( NULL == (g_pD3D = Direct3DCreate9( D3D_SDK_VERSION)) )
		return E_FAIL;
	D3DDISPLAYMODE d3ddm;

	if( FAILED( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT,
			&d3ddm)) )
	{
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));

	d3dpp.Windowed = FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	d3dpp.BackBufferWidth = d3ddm.Height;
	d3dpp.BackBufferHeight = d3ddm.Width;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D32;

	if(FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&g_pd3dDevice)))
	{
		return E_FAIL;
	}


	return S_OK;
}
void D3DRender::Render()
{

}