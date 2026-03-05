#include <windows.h>
#include <DirectXMath.h>
#include <iostream>

#include "Matrix.h"

using namespace std;
using namespace DirectX;

ostream& operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);
	os << "(" << dest.x << ", " << dest.y << ", "
		<< dest.z << ", " << dest.w << ")";
	return os;
}
ostream& operator<<(ostream& os, FXMMATRIX m)
{
	XMFLOAT4X4 f;
	XMStoreFloat4x4(&f, m);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			os <<f.m[i][j] << "\t";
		}
		os << endl;
	}
	return os;
}

ostream& operator<<(ostream& os, Matrix m)
{
	for (int i = 0; i < m.row; ++i)
	{
		for (int j = 0; j < m.col; ++j)
		{
			os << m.map[i][j] << " ";
		}
		os << endl;
	}

	return os;
}

int main()
{
	/*
	XMMATRIX A(	1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 2.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 4.0f, 0.0f,
				1.0f, 2.0f, 3.0f, 1.0f);
	XMMATRIX B = XMMatrixIdentity();
	XMMATRIX C = A * B;
	XMMATRIX D = XMMatrixTranspose(A);
	XMVECTOR det = XMMatrixDeterminant(A);
	XMMATRIX E = XMMatrixInverse(&det, A);
	XMMATRIX F = A * E;
	cout << "A = " << endl << A << endl;
	cout << "B = " << endl << B << endl;
	// A x 단위 행렬 = A
	cout << "C = A*B = " << endl << C << endl;
	// 전치 행렬
	cout << "D = transpose(A) = " << endl << D << endl;
	// 행렬식 SIMD로 계산 통일 하기위해 VECTOR로 반환 (실제 값은 x요소)
	cout << "det = determinant(A) = " << det << endl << endl;
	cout << "E = inverse(A) = " << endl << E << endl;
	cout << "F = A*E = " << endl << F << endl;
	cout << endl;
	*/

	// exercise 2-18
	cout << "*************Exercise 2-18*************" << endl;
	
	Matrix a = { {1, 2, 3} };

	cout << a << endl;

	Matrix b = a.Transpose();

	cout << b << endl;

	Matrix c = { {4, 5, 6} };
	Matrix d = a * b;

	cout << d << endl;

	

	return 0;
}
