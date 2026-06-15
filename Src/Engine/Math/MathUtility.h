#pragma once

#include <bit>
#include <cstdint>
#include <cfloat>
#include <numbers>
#include <random>

namespace Engine::Math {
    constexpr float Pi = std::numbers::pi_v<float>;
    constexpr float Epsilon = FLT_EPSILON;
    constexpr float MAX = FLT_MAX;
    constexpr float MIN = FLT_MIN;

    constexpr float Rad2Deg(float rad) { return rad * 180.0f / Pi; }
    constexpr float Deg2Rad(float deg) { return deg * Pi / 180.0f; }

    // 코드의 일관성을 위해 Min, Max 등 구현

    template <typename T>
    constexpr T Min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template <typename T>
    constexpr T Max(const T& a, const T& b)
    {
        return a > b ? a : b;
    }

    template <typename T>
    constexpr T Lerp(const T& a, const T& b, float t)
    {
        return a + (b - a) * t;
    }

    template <typename T>
    constexpr T Clamp(const T& value, const T& minValue, const T& maxValue)
    {
        return Max(minValue, Min(value, maxValue));
    }

    // 2D 벡터 (x, y)의 방향 각도를 [0, 2 * PI) 범위의 라디안으로 반환한다.
    inline float AngleFromXY(float x, float y)
    {
        float theta = std::atan2(y, x);

        if (theta < 0.0f)
        {
            theta += 2.0f * Pi;
        }

        return theta;
    }

    class FRandomStream
    {
    public:
        FRandomStream()
            : FRandomStream(std::random_device{}())
        {
        }
        // 같은 시드로 생성한 랜덤 스트림은 항상 같은 순서의 값을 만든다.
        explicit FRandomStream(uint32_t seed)
            : InitialSeed(seed), Seed(seed)
        {
        }

        // 현재 시드를 처음 시드로 되돌린다.
        void Reset()
        {
            Seed = InitialSeed;
        }

        // [0, 1) 범위의 난수를 반환한다.
        float RandF()
        {
            return GetFraction();
        }

        // [minValue, maxValue) 범위의 난수를 반환한다.
        float RandF(float minValue, float maxValue)
        {
            return minValue + RandF() * (maxValue - minValue);
        }

    private:
        /**
         * LCG 점화식으로 내부 시드를 다음 상태로 갱신한다.
         * 점화식: Seed = (Seed * a + c) mod 2^32
         * 하위비트일수록 주기가 짧다. (비트 k의 주기 = 2^(k+1))
         * 하위비트를 % 연산으로 직접 사용하면 패턴이 반복된다.
         * GetFraction()에서 >> 9로 하위 9비트를 제거하는 이유가 이것이다.
         */
        void MutateSeed()
        {
            Seed = Seed * 196314165u + 907633515u;
        }

        /**
         * 갱신된 시드의 상위 23비트를 IEEE 754 float 가수부로 재해석해
         * [0, 1) 범위의 난수를 생성한다.
         */
        float GetFraction()
        {
            MutateSeed();

            const uint32_t bits = 0x3F800000u | (Seed >> 9);
            const float result = std::bit_cast<float>(bits);

            return result - 1.0f;
        }

    private:
        uint32_t InitialSeed = 0;
        uint32_t Seed = 0;
    };
}
