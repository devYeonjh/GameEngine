#pragma once

#include <cmath>
#include "MathUtility.h"

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

		/**
		 * 벡터 스칼라 곱 연산
		 */
		Vector3 operator*(const float value) const
		{
			return Vector3(x * value, y * value, z * value);
		}

		/**
		 * 벡터 스칼라 곱 대입 연산
		 */
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
		Vector3 ComponentProduct(const Vector3& other) const
		{
			return Vector3(x * other.x, y * other.y, z * other.z);
		}

		float DotProduct(const Vector3& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		/**
		 * Dot Product
		 */
		float operator*(const Vector3& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3 CrossProduct(const Vector3& other) const
		{
			return Vector3(	y * other.z - z * other.y,
							z * other.x - x * other.z,
							x * other.y - y * other.x );
		}

		/**
		 * Cross Product
		 */
		Vector3 operator%(const Vector3& other) const
		{
			return Vector3(y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
		}

		void operator%=(const Vector3& other)
		{
			*this = CrossProduct(other);
		}

		/********************* Inline Method *********************/
		
		/** 벡터의 크기 */
		float Magnitude() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		/** 벡터 크기의 제곱 */
		float SquareMagnitude() const
		{
			return x * x + y * y + z * z;
		}

		/** 현재 벡터를 정규화 시킨 값으로 변환 */
		void Normalize()
		{
			float l = Magnitude();
			if (l > 0)
			{
				(*this) *= 1 / ((float)l);
			}
		}

		/** 정규화된 벡터 반환 */
		Vector3 Unit() const
		{
			Vector3 result = *this;
			result.Normalize();
			return result;
		}

		/**
		 * 벡터를 scale 값만큼 곱해서 추가
		 * scaled된 벡터를 따로 생성하지않고 직접 값에 추가
		 */
		void AddScaledVector(const Vector3& vector, float scale)
		{
			x += vector.x * scale;
			y += vector.y * scale;
			z += vector.z * scale;
		}

		void Invert()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		void Clear()
		{
			x = y = z = 0.f;
		}

		/** other Vector사이의 각도 */
		float Degree(const Vector3& other) const
		{
			float theta = std::acos(this->Unit() * other.Unit());
			return Rad2Deg(theta);
		}
	};

}