#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <vector>

class FWaves
{
public:
	FWaves() { };
	~FWaves() = default;

	UINT GetRowCount()const;
	UINT GetColumnCount()const;
	UINT GetVertexCount()const;
	UINT GetTriangleCount()const;

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& operator[](int i)const { return CurrSolution[i]; }

	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(UINT i, UINT j, float magnitude);

private:
	UINT NumRows = 0;
	UINT NumCols = 0;

	UINT VertexCount = 0;
	UINT TriangleCount = 0;

	// Simulation constants we can precompute.
	float K1 = 0.0f;
	float K2 = 0.0f;
	float K3 = 0.0f;

	float TimeStep = 0.0f;
	float SpatialStep = 0.0f;

	std::vector<DirectX::XMFLOAT3> PrevSolution;
	std::vector<DirectX::XMFLOAT3> CurrSolution;
};
