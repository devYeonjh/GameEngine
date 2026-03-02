#pragma once
#include <windows.h>

namespace App
{
    /**
     * QueryPerformanceCounter 기반의 고해상도 타이머를 사용합니다.
     * 프레임 번호, 델타 타임, 지수 이동 평균(EMA) FPS를 제공합니다.
     */
    class Timing
    {
    public:
        // ------------------------------------------
        // 프레임 정보
        // ------------------------------------------

        /** 현재까지 누적된 렌더 프레임 번호 (일시정지 시 증가하지 않음) */
        unsigned int FrameNumber = 0;

        /**
         * 직전 프레임의 소요 시간 (초 단위)
         *
         * 물리 시뮬레이션의 적분 스텝으로 사용합니다.
         * 과도하게 큰 값을 방지하기 위해 0.05f로 클램핑됩니다.
         */
        float DeltaTime = 0.0f;

        /**
         * 지수 이동 평균(EMA)으로 계산된 초당 프레임 수
         *
         * 최근 100프레임을 기준으로 계산되며 2프레임 이후부터 유효합니다.
         */
        float FPS = 0.0f;

        /**
         * EMA 계산에 사용되는 평균 프레임 시간 (초 단위)
         * FPS = 1.0f / AverageFrameDuration
         */
        double AverageFrameDuration = 0.0;

        /** 렌더링 일시정지 여부 (true이면 FrameNumber가 증가하지 않음) */
        bool IsPaused = false;

    public:
        /**
         * 전역 Timing 인스턴스를 반환합니다.
         *
         * Init() 호출 이전에 사용하면 nullptr 역참조가 발생합니다.
         */
        static Timing& Get();

        // ------------------------------------------
        // 생명주기
        // ------------------------------------------

        /**
         * 타이밍 시스템을 초기화합니다.
         *
         * QueryPerformanceFrequency로 타이머 주파수를 측정하고
         * 전역 인스턴스를 생성합니다.
         * 애플리케이션 시작 시 반드시 1회 호출해야 합니다.
         */
        static void Init();

        /**
         * 타이밍 시스템을 종료하고 전역 인스턴스를 해제합니다.
         *
         * 애플리케이션 종료 시 Release() 계열 함수들과 함께 호출합니다.
         */
        static void Deinit();

        // ------------------------------------------
        // 프레임 갱신
        // ------------------------------------------

        /**
         * 프레임 타이밍 정보를 갱신합니다.
         *
         * 매 프레임 루프의 시작 부분에서 1회 호출해야 합니다.
         * DeltaTime, FPS, AverageFrameDuration, FrameNumber를 갱신합니다.
         */
        void Update();

        // ------------------------------------------
        // 편의 조회 함수
        // ------------------------------------------

        /**
         * 직전 프레임의 소요 시간을 초 단위로 반환합니다.
         *
         * DeltaTime의 편의 래퍼이며 물리 적분 스텝에 직접 사용 가능합니다.
         */
        float GetDeltaTime() const { return DeltaTime; }

        /**
         * EMA 기반 초당 프레임 수를 반환합니다.
         */
        float GetFPS() const { return FPS; }

    private:
        // 외부에서 인스턴스를 직접 생성하지 못하도록 생성자를 숨깁니다.
        Timing() = default;
        Timing(const Timing&) = delete;
        Timing& operator=(const Timing&) = delete;

        /** QueryPerformanceFrequency 결과 (틱 → 초 변환에 사용) */
        LARGE_INTEGER Frequency = {};

        /** 직전 프레임 종료 시점의 타이머 카운터 값 */
        LARGE_INTEGER LastFrameTimestamp = {};

        /** 전역 인스턴스 포인터 */
        static Timing* Instance;
    };
}