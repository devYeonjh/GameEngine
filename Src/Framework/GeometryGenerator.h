#pragma once

#include "pch.h"

namespace Framework
{
	class FGeometryGenerator
	{
	public:
		struct FVertex
		{
			FVertex() {}
			FVertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
				: Position(p), Normal(n), TangentU(t), TexC(uv) {
			}
			FVertex(
				float px, float py, float pz,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				float u, float v)
				: Position(px, py, pz), Normal(nx, ny, nz),
				TangentU(tx, ty, tz), TexC(u, v) {
			}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 TangentU;
			DirectX::XMFLOAT2 TexC;
		};

		struct FMeshData
		{
			std::vector<FVertex> Vertices;
			std::vector<std::uint32_t> Indices;
		};

		///<summary>
		/// 주어진 크기의 박스를 원점을 중심으로 생성
		///</summary>
		void CreateBox(float width, float height, float depth, FMeshData& meshData);

		///<summary>
		/// 주어진 반지름을 가진 구를 원점을 중심으로 생성
		/// slices와 stacks 매개변수는 테셀레이션 정도를 제어한다.
		///</summary>
		void CreateSphere(float radius, UINT sliceCount, UINT stackCount, FMeshData& meshData);

		///<summary>
		/// C주어진 반지름을 가진 지오스피어를 원점을 중심으로 생성한다.
		/// depth는 테셀레이션 레벨을 제어한다.
		///</summary>
		void CreateGeosphere(float radius, UINT numSubdivisions, FMeshData& meshData);

		///<summary>
		//// y축과 평행하고, 원점을 중심으로 하는 원기둥을 생성한다.
		/// 아래쪽 반지름과 위쪽 반지름은 서로 다를 수 있으며,
		/// 이 경우 완전한 원기둥이 아니라 원뿔 또는 절두원뿔 형태가 될 수 있다.
		/// slices와 stacks 매개변수는 테셀레이션 정도를 제어한다.
		///</summary>
		void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, FMeshData& meshData);

		///<summary>
		/// xz 평면 위에 m행 n열의 격자를 생성한다.
		/// 격자는 지정한 width와 depth를 가지며 원점을 중심으로 배치된다.
		///</summary>
		void CreateGrid(float width, float depth, UINT m, UINT n, FMeshData& meshData);

		///<summary>
		/// NDC 좌표계에서 화면 전체를 덮는 사각형을 생성한다.
		/// 후처리 효과에 유용하다.
		///</summary>
		void CreateFullscreenQuad(FMeshData& meshData);

	private:
		void Subdivide(FMeshData& meshData);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, FMeshData& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, FMeshData& meshData);
	};
}
