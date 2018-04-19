#include "stdafx.h"
#include "Timer.h"

//nothing to do in constructor & destructor
Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::Initialize()
{
	GetTime(); //call get time to fill last frame time variable with a value ( means start timer)
	return ;
}

//function that returns delta time
float Timer::GetTime()
{
	LARGE_INTEGER tEnd, tDif, frequency; 

	// read the performance counter at the end
	QueryPerformanceCounter(&tEnd);
	//frequency is system dependent so I have to obtain it for calculations
	QueryPerformanceFrequency(&frequency);
	// calculate microsocends using LARGE_INTEGERS
	tDif.QuadPart = tEnd.QuadPart - m_tLastFrame.QuadPart;
	tDif.QuadPart *= 1000000;
	tDif.QuadPart /= frequency.QuadPart;

	m_tLastFrame = tEnd;

	return (float)(((double)tDif.LowPart) / 1000000); // use doubles to avoid rounding issues
}