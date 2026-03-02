#include "App/Timing.h"
#include <cassert>

using namespace App;

// 전역 인스턴스 포인터 초기화
Timing* Timing::Instance = nullptr;

Timing& Timing::Get()
{
    assert(Instance && "Timing::Init()을 먼저 호출해야 합니다.");
    return *Instance;
}

void Timing::Init()
{
    if (Instance) return;

    Instance = new Timing();

    // 타이머 주파수 측정 (초기화 1회)
    QueryPerformanceFrequency(&Instance->Frequency);

    // 기준 시점 설정
    QueryPerformanceCounter(&Instance->LastFrameTimestamp);

    Instance->FrameNumber = 0;
    Instance->DeltaTime = 0.0f;
    Instance->FPS = 0.0f;
    Instance->AverageFrameDuration = 0.0;
    Instance->IsPaused = false;
}

void Timing::Deinit()
{
    delete Instance;
    Instance = nullptr;
}

void Timing::Update()
{
    if (!Instance) return;

    // 현재 프레임 시점 측정
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    // 직전 프레임과의 경과 시간 계산 (초 단위)
    double elapsed = static_cast<double>(now.QuadPart - LastFrameTimestamp.QuadPart)
        / static_cast<double>(Frequency.QuadPart);

    // 기준 시점 갱신
    LastFrameTimestamp = now;

    // 물리 시뮬레이션 불안정을 방지하기 위해 상한 클램핑
    // (디버거 중단 등으로 매우 긴 프레임이 발생하는 경우 대응)
    DeltaTime = (elapsed > 0.05) ? 0.05f : static_cast<float>(elapsed);

    // 일시정지 중에는 프레임 번호를 증가시키지 않음
    if (!IsPaused)
    {
        FrameNumber++;
    }

    // EMA(지수 이동 평균) FPS 갱신 - 2프레임 이후부터 유효
    // 원본 Cyclone: RWA over 100 frames (0.99 / 0.01 비율)
    if (FrameNumber > 1)
    {
        if (AverageFrameDuration <= 0.0)
        {
            AverageFrameDuration = elapsed;
        }
        else
        {
            AverageFrameDuration = AverageFrameDuration * 0.99 + elapsed * 0.01;
        }

        FPS = (AverageFrameDuration > 0.0)
            ? static_cast<float>(1.0 / AverageFrameDuration)
            : 0.0f;
    }
}
