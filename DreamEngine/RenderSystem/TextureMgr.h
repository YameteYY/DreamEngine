#ifndef __TEXTUREMGR_H__
#define __TEXTUREMGR_H__
#include "D3DHeader.h"
#include <map>

class TextureMgr
{
public:
	static TextureMgr* Instance()
	{
		if(m_pInstance == 0)
			m_pInstance = new TextureMgr();
		return m_pInstance;
	}
	IDirect3DTexture9* GetTexture(std::string str);
protected:
	static TextureMgr* m_pInstance;
	TextureMgr(){}
	std::map<std::string,IDirect3DTexture9*> mTexurePool;
};

#endif