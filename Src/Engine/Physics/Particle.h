#pragma once

#include "Math/Vector3.h"

namespace Engine {
	/**
	 * 위치와 속도는 있지만 방향성은 없는 단순한 오브젝트
	 * 뉴턴의 운동법칙 적용
	 */
	class Particle
	{
	protected:
		/** 월드 공간상의 위치 */
		Vector3 position;

		/** 월드 공간상에서 속도 */
		Vector3 velocity;

		/** particle의 가속도, 주로 중력가속도에 사용 */
		Vector3 acceleration;

		/** 
		 * 선형 운동에 적용되는 감쇠량
		 * 적분기의 수치적 불안정을 통해 추가된 에너지를 제거하기 위함
		 * damping이 0이면 업데이트 이후 속도가 완전히 사라짐 1이면 모든 속도 유지(감쇠가 없는것과 동일)
		 * 물체가 저항을 받는 것처럼 보이게 하고 싶지 않다면, 1에 가깝지만 1보다 작은 값이 최적
		 */
		float damping;

		/**
		 * 질량의 역수를저장
		 * 적분이 더 간단해지고 (a = 1/m * F), 0으로 나누는것을 방지
		 * 실시간 시뮬레이션에서는 무한질량이 zero질량보다 더 유용하게 사용됨
		 * (만약 m이 무한대면 a는 0, m이 0일 수는 없음)
		 */
		float inverseMass;
		
	public:
		/**
		 * 지정된 시간만큼 particle을 앞으로 이동
		 * Newton-Euler 적분법을 사용하며 이는 실제 적분값에 대한 선형 근사치
		 * (매 프레임 구간에선 속도와 가속도가 변하지 않는다고 가정)
		 */
		void Integrate(float duration);

		/**
		 * Exercise 3.3
		 * 운동 에너지 반환
		 * E = 1/2 * m * |v|^2
		 */
		float GetKineticEnergy();

		/**
		 * mass Setter
		 *
		 * @param mass 0이 될 수 없음. 너무 작은 질량은 시뮬레이션을 불안정하게 만듬.
		 *
		 * @warning 질량을 변경하면 내부적인 값 들이 무효화됨.
		 * 질량을 변경한 뒤 @Integrate() 혹은 CalculateInternals()를 호출해서
		 * 내부 데이터를 갱신해야 함.
		 */
		void setMass(const float mass);

		/** mass Getter */
		float getMass() const;

		/**
		 * inverseMass Setter
		 *
		 * @param inverseMass 0인 경우 무한질량(움직이지 않는) 물체
		 *
		 * @warning 질량을 변경하면 내부적인 값 들이 무효화됨.
		 * 질량을 변경한 뒤 @Integrate() 혹은 CalculateInternals()를 호출해서
		 * 내부 데이터를 갱신해야 함.
		 */
		void setInverseMass(const float inverseMass);

		/** inverseMass Getter */
		float getInverseMass() const;

	};

}