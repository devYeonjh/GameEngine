#include <assert.h>
#include "Physics/Particle.h"

using namespace Engine::Physics;

void Particle::Integrate(float duration)
{
	// 무한질량인 경우 적분하지 않음 (정지상태 유지)
	if (inverseMass <= 0.0f) return;

	assert(duration > 0.f);

	// 선형 위치 업데이트
	position.AddScaledVector(velocity, duration);

	// 힘으로 부터 가속도 업데이트 (힘을 생성할때 코드 추가)
	Math::FVector resultingAcc = acceleration;

	// 선형 가속도 업데이트
	velocity.AddScaledVector(resultingAcc, duration);

	// 항력 적용
	velocity *= std::pow(damping, duration);

	// 힘 제거
	//ClearAccumulator();
}

float Particle::GetKineticEnergy()
{
	// E = 1/2 * m * |v|^2;
	float KineticEnergy = getMass() * velocity.SquareMagnitude() * 0.5f;

	return KineticEnergy;
}

void Particle::setMass(const float mass)
{
	assert(mass != 0);
	Particle::inverseMass = 1.0f / mass;
}

float Particle::getMass() const
{
	if (inverseMass == 0)
		return Math::MAX;
	else
		return 1.0f / inverseMass;
}

void Particle::setInverseMass(const float inverseMass)
{
	Particle::inverseMass = inverseMass;
}

float Particle::getInverseMass() const
{
	return inverseMass;
}
