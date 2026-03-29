#pragma once

#include <float.h>

namespace Engine::Math {
	constexpr float Pi = 3.1415927f;
	constexpr float Rad2Deg(float rad) { return rad * 180.0f / Pi; }
	constexpr float Deg2Rad(float deg) { return deg * Pi / 180.0f; }
	constexpr float Epsilon = FLT_EPSILON;
	constexpr float MAX = FLT_MAX;
}