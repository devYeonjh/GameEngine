#include "pch.h"
#include "Timer.h"

using namespace Framework;

FTimer::FTimer()
	: SecondsPerCount(0.0), DeltaTime(-1.0), BaseTime(0),
	PausedTime(0), PrevTime(0), CurrTime(0), bIsStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0 / (double)countsPerSec;
}

// Reset()이 호출된 이후로 경과한 총 시간을 반환.
// 타이머가 정지된 동안의 시간은 포함하지 않음.
float FTimer::TotalTime()const
{
	// 정지 상태라면, 정지된 이후 경과한 시간은 계산하지 않음.
	// 또한 이전에 이미 일시정지가 있었다면,
	// mStopTime - mBaseTime 구간에 일시정지 시간이 포함되므로 이를 빼줘야 함.
	if (bIsStopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}

	// mCurrTime - mBaseTime 구간에는 일시정지 시간이 포함되어 있으므로 이를 빼줘야 함.
	else
	{
		return (float)(((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

float FTimer::GetDeltaTime() const
{
	return (float)DeltaTime;
}

void FTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	BaseTime = currTime;
	PrevTime = currTime;
	StopTime = 0;
	bIsStopped = false;
}

void FTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// 정지 상태와 시작 상태 사이에 경과한 시간을 누적.
	if (bIsStopped)
	{
		PausedTime += (startTime - StopTime);

		PrevTime = startTime;
		StopTime = 0;
		bIsStopped = false;
	}
}

void FTimer::Stop()
{
	if (!bIsStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		StopTime = currTime;
		bIsStopped = true;
	}
}

void FTimer::Tick()
{
	if (bIsStopped)
	{
		DeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	CurrTime = currTime;

	// 이번 프레임과 이전 프레임 사이의 시간 차이.
	DeltaTime = (CurrTime - PrevTime) * SecondsPerCount;

	// 다음 프레임을 위해 준비.
	PrevTime = CurrTime;

	// 음수가 되지 않도록 강제.
	// DXSDK의 CDXUTTimer에 따르면, CPU가 절전 모드로 진입하거나.
	// 다른 프로세서로 작업이 넘어갈 경우 mDeltaTime이 음수가 될 수 있음.
	if (DeltaTime < 0.0)
	{
		DeltaTime = 0.0;
	}
}