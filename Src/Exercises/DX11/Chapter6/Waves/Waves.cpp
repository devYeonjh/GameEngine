#include "Waves.h"
#include <algorithm>
#include <vector>
#include <cassert>

using DirectX::XMFLOAT3;

UINT FWaves::GetRowCount()const
{
	return NumRows;
}

UINT FWaves::GetColumnCount()const
{
	return NumCols;
}

UINT FWaves::GetVertexCount()const
{
	return VertexCount;
}

UINT FWaves::GetTriangleCount()const
{
	return TriangleCount;
}

void FWaves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	NumRows = m;
	NumCols = n;

	VertexCount = m * n;
	TriangleCount = (m - 1) * (n - 1) * 2;

	TimeStep = dt;
	SpatialStep = dx;

	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);
	K1 = (damping * dt - 2.0f) / d;
	K2 = (4.0f - 8.0f * e) / d;
	K3 = (2.0f * e) / d;

	// Init()이 다시 호출될 경우를 대비한 처리.
	PrevSolution.resize(m * n);
	CurrSolution.resize(m * n);

	// 시스템 메모리에 그리드 정점을 생성.
	float halfWidth = (n - 1) * dx * 0.5f;
	float halfDepth = (m - 1) * dx * 0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			PrevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			CurrSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
		}
	}
}

void FWaves::Update(float dt)
{
	static float t = 0;

	// 시간을 누적.
	t += dt;

	// 지정된 타임스텝마다 시뮬레이션을 갱신.
	if (t >= TimeStep)
	{
		// 내부 점들만 갱신하며, 경계는 0으로 고정(zero boundary condition).
		for (DWORD i = 1; i < NumRows - 1; ++i)
		{
			for (DWORD j = 1; j < NumCols - 1; ++j)
			{
				// 이번 갱신이 끝나면 이전(prev) 버퍼는 버려질 것이므로,
				// 새로 계산한 값을 그 버퍼에 바로 덮어쓴다.
				// prev_ij 값은 다시 필요하지 않고 대입이 가장 마지막에 일어나므로
				// 같은 요소를 읽고 쓰는(in-place) 방식이 안전하다.

				// j는 x축, i는 z축 인덱스를 의미한다: h(x_j, z_i, t_k)
				// 또한 +z축은 화면상 "아래쪽"을 향하므로,
				// 행(row) 인덱스가 아래로 증가하는 것과 일치시키기 위함이다.

				PrevSolution[i * NumCols + j].y =
					K1 * PrevSolution[i * NumCols + j].y +
					K2 * CurrSolution[i * NumCols + j].y +
					K3 * (CurrSolution[(i + 1) * NumCols + j].y +
						CurrSolution[(i - 1) * NumCols + j].y +
						CurrSolution[i * NumCols + j + 1].y +
						CurrSolution[i * NumCols + j - 1].y);
			}
		}

		// 방금 prev 버퍼에 새 데이터를 덮어썼으므로,
		// 이 데이터가 현재(curr) 해가 되어야 하고,
		// 기존 curr 해는 새로운 prev 해가 되어야 한다.
		std::swap(PrevSolution, CurrSolution);

		t = 0.0f; // 시간 초기화
	}
}

void FWaves::Disturb(UINT i, UINT j, float magnitude)
{
	// 경계에는 영향을 주지 않는다.
	assert(i > 1 && i < NumRows - 2);
	assert(j > 1 && j < NumCols - 2);

	float halfMag = 0.5f * magnitude;

	// (i, j) 정점의 높이와 그 주변 정점들을 교란시킨다.
	CurrSolution[i * NumCols + j].y += magnitude;
	CurrSolution[i * NumCols + j + 1].y += halfMag;
	CurrSolution[i * NumCols + j - 1].y += halfMag;
	CurrSolution[(i + 1) * NumCols + j].y += halfMag;
	CurrSolution[(i - 1) * NumCols + j].y += halfMag;
}
