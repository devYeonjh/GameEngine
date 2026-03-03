#include <windows.h>
#include <DirectXMath.h>
#include <iostream>

// 실습에서만 사용
using namespace std;
using namespace DirectX;

// XMVECTOR를 << 연산자로 출력하기 위한 오버로딩
ostream& operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);
	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int main()
{
	
	/*XMVECTOR p = XMVectorZero();
	XMVECTOR q = XMVectorSplatOne();
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorReplicate(-2.0f);
	XMVECTOR w = XMVectorSplatZ(u);

	cout << "p = " << p << endl;
	cout << "q = " << q << endl;
	cout << "u = " << u << endl;
	cout << "v = " << v << endl;
	cout << "w = " << w << endl;*/

	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	cout.setf(ios_base::boolalpha);

	// 덧셈
	XMVECTOR a = u + v;

	// 뺄셈
	XMVECTOR b = u - v;

	// 스칼라값 곱셈
	XMVECTOR c = 10.0f * u;

	// 벡터의 크기
	XMVECTOR L = XMVector3Length(u);

	// 정규화
	XMVECTOR d = XMVector3Normalize(u);

	// 내적
	XMVECTOR s = XMVector3Dot(u, v);

	// 외적
	XMVECTOR e = XMVector3Cross(u, v);

	// 직교성분과 수직성분
	XMVECTOR projW;
	XMVECTOR perpW;
	XMVector3ComponentsFromNormal(&projW, &perpW, w, n);

	// 직교성분과 수직성분의 합
	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;

	// 직교성분과 수직성분 사이의 각도
	XMVECTOR angleVec = XMVector3AngleBetweenVectors(projW, perpW);
	float angleRadians = XMVectorGetX(angleVec);
	float angleDegrees = XMConvertToDegrees(angleRadians);

	cout << "u :\t\t\t" << u << endl;
	cout << "v :\t\t\t" << v << endl;
	cout << "w :\t\t\t" << w << endl;
	cout << "n :\t\t\t" << n << endl;
	cout << "a = u + v :\t\t" << a << endl;
	cout << "b = u - v :\t\t" << b << endl;
	cout << "c = 10 * u :\t\t" << c << endl;
	cout << "d = u / ||u|| :\t\t" << d << endl;
	cout << "e = u x v :\t\t" << e << endl;
	cout << "L = ||u|| :\t\t" << L << endl;
	cout << "s = u.v :\t\t" << s << endl;
	cout << "projW :\t\t\t" << projW << endl;
	cout << "perpW :\t\t\t" << perpW << endl;
	cout << "projW + perpW == w :\t" << equal << endl;
	cout << "projW + perpW != w :\t" << notEqual << endl;
	cout << "angle :\t\t\t" << angleDegrees << endl;

	return 0;
}