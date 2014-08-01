#include "D3DFont.h"

D3DFont* D3DFont::m_pInstance = 0;

void D3DFont::Init(LPDIRECT3DDEVICE9 Device)
{
	g_pDevice = Device;
	D3DXFONT_DESC           font_desc;
	// create the font
	ZeroMemory(&font_desc, sizeof(font_desc));
	// set font descripter
	strcpy(font_desc.FaceName, "Arial");
	font_desc.Height = 32;

	// Creates a font object indirectly for both a device and a font
	D3DXCreateFontIndirect(g_pDevice, &font_desc, &mFont);
}
void D3DFont::Draw(const char* str)
{
	 RECT rect = { 0, 0, 200, 200 };
	 mFont->DrawTextA(NULL, str, -1,&rect, DT_LEFT | DT_TOP, 0xFFFFFFFF);
}