#ifndef __D3DFONT_H__
#define __D3DFONT_H__
#include "D3DHeader.h"

class D3DFont
{
public:
	static D3DFont* Instance()
	{
		if(m_pInstance == 0)
		{
			m_pInstance = new D3DFont();
		}
		return m_pInstance;
	}
	void Init(LPDIRECT3DDEVICE9 Device);
	void Draw(const char* str);
private:
	ID3DXFont* mFont;
	
	LPDIRECT3DDEVICE9 g_pDevice;
	D3DFont(){}
	static D3DFont* m_pInstance;
};


#endif