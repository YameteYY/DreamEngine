#include "TimeManager.h"

TimeManager* TimeManager::m_pInstance = 0;
TimeManager::TimeManager()
{
	g_fps = 0;
	frameCount = 0;//֡��  
	currentTime =0.0f;//��ǰʱ��  
	lastTime = timeGetTime()*0.001f;//����ʱ��  
}
void TimeManager::Update()
{
	frameCount++;//ÿ����һ��Get_FPS()������֡������1  
	currentTime = timeGetTime()*0.001f;//��ȡϵͳʱ�䣬����timeGetTime�������ص����Ժ���Ϊ��λ��ϵͳʱ�䣬������Ҫ����0.001���õ���λΪ���ʱ��  

	//�����ǰʱ���ȥ����ʱ�������1���ӣ��ͽ���һ��FPS�ļ���ͳ���ʱ��ĸ��£�����֡��ֵ����  
	if(currentTime - lastTime > 1.0f) //��ʱ�������1����  
	{  
		g_fps = (float)frameCount /(currentTime - lastTime);//������1���ӵ�FPSֵ  
		lastTime = currentTime; //����ǰʱ��currentTime��������ʱ��lastTime����Ϊ��һ��Ļ�׼ʱ��  
		frameCount    = 0;//������֡��frameCountֵ����  
	}
}