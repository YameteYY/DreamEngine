#ifndef __TIMEMANAGER_H__
#define __TIMEMANAGER_H__
#include <windows.h> 
#include <stdio.h> 

class TimeManager
{
public:
	static TimeManager* Instance()
	{
		if(m_pInstance == 0)
		{
			m_pInstance = new TimeManager();
		}
		return m_pInstance;
	}
	void Update();
	float GetFPS();
private:
	float g_fps;
	int    frameCount;//֡��  
	float  currentTime;//��ǰʱ��  
	float  lastTime;//����ʱ��  
	TimeManager();
	static TimeManager* m_pInstance;
};
inline float TimeManager::GetFPS()
{
	return g_fps;
}

#endif