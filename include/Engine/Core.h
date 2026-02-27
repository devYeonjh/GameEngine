#pragma once

#include <cmath>

namespace Engine {
	/**
	 * 3차원 벡터
	 */
	struct Vector3
	{
	public:
		float x, y, z;

	private:
		/**
		 * 성능 향상을 위한 추가 변수
		 * 일반적으로 부동소수점 값 4개가 3개보다 메모리에 효율적으로 배치됨
		 */
		float pad;

	public:
		Vector3() : x(0.f), y(0.f), z(0.f) {}

		Vector3(const float x, const float y, const float z)
			: x(x), y(y), z(z) {
		}

		const static Vector3 GRAVITY;
		const static Vector3 HIGH_GRAVITY;
		const static Vector3 UP;
		const static Vector3 RIGHT;
		const static Vector3 OUT_OF_SCREEN;
		const static Vector3 X;
		const static Vector3 Y;
		const static Vector3 Z;

	public:
		/********************* Operator Overloading *********************/
		float operator[](unsigned i) const
		{
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}

		float& operator[](unsigned i)
		{
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}

		Vector3 operator*(const float value) const
		{
			return Vector3(x * value, y * value, z * value);
		}

		void operator*=(const float value)
		{
			x *= value;
			y *= value;
			z *= value;
		}

		Vector3 operator+(const Vector3& other) const
		{
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		void operator+=(const Vector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}

		Vector3 operator-(const Vector3& other) const
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		void operator-=(const Vector3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}

		bool operator==(const Vector3& other) const
		{
			return x == other.x &&
				y == other.y &&
				z == other.z;
		}

		bool operator!=(const Vector3& other) const
		{
			return !(*this == other);
		}

		/********************* Multiplying Vector *********************/
		Vector3 componentProduct(const Vector3& other) const
		{
			return Vector3(x * other.x, y * other.y, z * other.z);
		}

		float dotProduct(const Vector3& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3 crossProduct(const Vector3& other) const
		{
			return Vector3(	y * other.z - z * other.y,
							z * other.x - x * other.z,
							x * other.y - y * other.x );
		}

		/********************* Inline Method *********************/
		float Magnitude() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		float SquareMagntude() const
		{
			return x * x + y * y + z * z;
		}

		void Normalize()
		{
			float l = Magnitude();
			if (l > 0)
			{
				(*this) *= ((float)l) / 1;
			}
		}

		void AddScaledVector(const Vector3& vector, float scale)
		{
			x += vector.x * scale;
			y += vector.y * scale;
			z += vector.z * scale;
		}

		Vector3 unit() const
		{
			Vector3 result = *this;
			result.Normalize();
			return result;
		}

		void Invert()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		void clear()
		{
			x = y = z = 0.f;
		}

	};

}