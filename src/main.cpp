#include "Engine/Engine.h"
#include <iostream>

using namespace Engine;

int main()
{
	// Exercise 2.6
	// c : b에서 a방향으로 정사영 했을때의 스칼라 크기 (a가 단위벡터 이므로)
	// a*c : b에서 a방향 성분만 뽑아낸 벡터, b의 a방향 projection 벡터
	// d = b - a*c 식의 기하학적 의미
	// -> b = d + a*c
	// -> a*c는 b에서 a방향의 성분
	// -> d는 b에서 이를 제외한 성분
	// -> 그러므로 d는 a*c에 대해서 수직

	Vector3 a(0, 1, 1);
	Vector3 b(1, 2, 3);

	a.Normalize(); // a *= 1 / sqrt(2);

	float c = a.DotProduct(b);
	std::cout << c << std::endl; // c = 5/sqrt(2)

	Vector3 d = b - a * c;

	std::cout << d.x << "," << d.y << "," << d.z << std::endl;

	float deg = d.Degree(a);
	std::cout << deg << std::endl; //deg = 90

	return 0;
}