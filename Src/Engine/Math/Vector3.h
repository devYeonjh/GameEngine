#pragma once

#include <cmath>
#include "MathUtility.h"

namespace Engine::Math {
	/**
	 * 3차원 벡터
	 */
	struct FVector
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
		FVector() : x(0.f), y(0.f), z(0.f) {}

		FVector(const float x, const float y, const float z)
			: x(x), y(y), z(z) {
		}

		const static FVector GRAVITY;
		const static FVector HIGH_GRAVITY;
		const static FVector UP;
		const static FVector RIGHT;
		const static FVector OUT_OF_SCREEN;
		const static FVector X;
		const static FVector Y;
		const static FVector Z;

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
		FVector operator*(const float value) const
		{
			return FVector(x * value, y * value, z * value);
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

		FVector operator+(const FVector& other) const
		{
			return FVector(x + other.x, y + other.y, z + other.z);
		}

		void operator+=(const FVector& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}

		FVector operator-(const FVector& other) const
		{
			return FVector(x - other.x, y - other.y, z - other.z);
		}

		void operator-=(const FVector& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}

		bool operator==(const FVector& other) const
		{
			return x == other.x &&
				y == other.y &&
				z == other.z;
		}

		bool operator!=(const FVector& other) const
		{
			return !(*this == other);
		}

		/********************* Multiplying Vector *********************/
		FVector ComponentProduct(const FVector& other) const
		{
			return FVector(x * other.x, y * other.y, z * other.z);
		}

		float DotProduct(const FVector& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		/**
		 * Dot Product
		 */
		float operator*(const FVector& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		FVector CrossProduct(const FVector& other) const
		{
			return FVector(	y * other.z - z * other.y,
							z * other.x - x * other.z,
							x * other.y - y * other.x );
		}

		/**
		 * Cross Product
		 */
		FVector operator%(const FVector& other) const
		{
			return FVector(y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
		}

		void operator%=(const FVector& other)
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
		FVector Unit() const
		{
			FVector result = *this;
			result.Normalize();
			return result;
		}

		/**
		 * 벡터를 scale 값만큼 곱해서 추가
		 * scaled된 벡터를 따로 생성하지않고 직접 값에 추가
		 */
		void AddScaledVector(const FVector& vector, float scale)
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
		float Degree(const FVector& other) const
		{
			float theta = std::acos(this->Unit() * other.Unit());
			return Rad2Deg(theta);
		}
	};

}