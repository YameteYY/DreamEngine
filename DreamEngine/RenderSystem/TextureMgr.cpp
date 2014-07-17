#include "TextureMgr.h"
#include "D3DRender.h"

TextureMgr* TextureMgr::m_pInstance = 0;

IDirect3DTexture9* TextureMgr::GetTexture(std::string str)
{
	std::map<std::string,IDirect3DTexture9*>::iterator itr;
	itr = mTexurePool.find(str);
	if(itr == mTexurePool.end())
	{
		LPDIRECT3DDEVICE9 Device = D3DRender::Instance()->GetDevice();
		IDirect3DTexture9* tex = 0;
		D3DXCreateTextureFromFile(
			Device,
			str.c_str(),
			&tex);
		mTexurePool.insert(std::pair<std::string,IDirect3DTexture9*>(str,tex));
		return tex;
	}
	return itr->second;
}