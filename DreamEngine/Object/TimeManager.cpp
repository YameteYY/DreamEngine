#include "TimeManager.h"

TimeManager* TimeManager::m_pInstance = 0;
TimeManager::TimeManager()
{
	g_fps = 0;
	frameCount = 0;//帧数  
	currentTime =0.0f;//当前时间  
	lastTime = timeGetTime()*0.001f;//持续时间  
}
void TimeManager::Update()
{
	frameCount++;//每调用一次Get_FPS()函数，帧数自增1  
	currentTime = timeGetTime()*0.001f;//获取系统时间，其中timeGetTime函数返回的是以毫秒为单位的系统时间，所以需要乘以0.001，得到单位为秒的时间  

	//如果当前时间减去持续时间大于了1秒钟，就进行一次FPS的计算和持续时间的更新，并将帧数值清零  
	if(currentTime - lastTime > 1.0f) //将时间控制在1秒钟  
	{  
		g_fps = (float)frameCount /(currentTime - lastTime);//计算这1秒钟的FPS值  
		lastTime = currentTime; //将当前时间currentTime赋给持续时间lastTime，作为下一秒的基准时间  
		frameCount    = 0;//将本次帧数frameCount值清零  
	}
}