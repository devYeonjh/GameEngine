#include "D3DApp.h"
#include "DebugLog.h"
#include "GeometryGenerator.h"
#include "Math/MathUtility.h"
#include "DemoPathUtil.h"

using namespace Framework;
using namespace DirectX;
using Debug::DebugLog;
using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

struct FVertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

struct FPerObjectConstants
{
	XMFLOAT4X4 WorldViewProjection;
};

class FShapesApp : public FD3DApp
{
public:
	FShapesApp(HINSTANCE hInstance);
	~FShapesApp() override = default;

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void DrawShapes(const XMFLOAT4X4& WorldMatrix, const XMMATRIX& ViewProj, UINT IndexCount, UINT IndexOffset, INT VertexOffset);
	void CreateGeometryBuffer();
	void CreateShader();
	void CreateConstantBuffer();

private:
	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;
	ComPtr<ID3D11Buffer> PerObjectConstantBuffer;

	ComPtr<ID3D11VertexShader> VertexShader;
	ComPtr<ID3D11PixelShader> PixelShader;
	ComPtr<ID3D11InputLayout> InputLayout;

	ComPtr<ID3D11RasterizerState> WireframeRasterizerState;

	XMFLOAT4X4 SphereWorld[10];
	XMFLOAT4X4 CylinderWorld[10];

	XMFLOAT4X4 BoxWorld;
	XMFLOAT4X4 GridWorld;
	XMFLOAT4X4 CenterSphereWorld;

	int BoxVertexOffset = 0;
	int GridVertexOffset = 0;
	int SphereVertexOffset = 0;
	int CylinderVertexOffset = 0;

	UINT BoxIndexOffset = 0;
	UINT GridIndexOffset = 0;
	UINT SphereIndexOffset = 0;
	UINT CylinderIndexOffset = 0;

	UINT BoxIndexCount = 0;
	UINT GridIndexCount = 0;
	UINT SphereIndexCount = 0;
	UINT CylinderIndexCount = 0;

	XMFLOAT4X4 ViewMatrix;
	XMFLOAT4X4 ProjectionMatrix;

	// 카메라의 수평 방향 각도
	float CameraTheta;
	// 카메라의 수직 방향 각도
	float CameraPhi;
	// 원점에서 카메라까지의 거리
	float CameraRadius;

	POINT LastMousePos;
};

FShapesApp::FShapesApp(HINSTANCE hInstance)
	: FD3DApp(hInstance),
	CameraTheta(1.5f * Engine::Math::Pi),
	CameraPhi(0.1f * Engine::Math::Pi),
	CameraRadius(15.0f),
	LastMousePos()
{
	MainWndCaption = L"Shapes Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&GridWorld, I);
	XMStoreFloat4x4(&ViewMatrix, I);
	XMStoreFloat4x4(&ProjectionMatrix, I);

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&BoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&CenterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&CylinderWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&CylinderWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&SphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&SphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}
}

bool FShapesApp::Init()
{
	if (!FD3DApp::Init())
		return false;

	CreateGeometryBuffer();
	CreateShader();
	CreateConstantBuffer();

	// 레스터라이저 생성
	D3D11_RASTERIZER_DESC WireframeDesc{};
	WireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	WireframeDesc.CullMode = D3D11_CULL_BACK;
	WireframeDesc.FrontCounterClockwise = false;
	WireframeDesc.DepthClipEnable = true;

	ThrowIfFailed(Device->CreateRasterizerState(&WireframeDesc, WireframeRasterizerState.GetAddressOf()));

	return true;
}

void FShapesApp::OnResize()
{
	FD3DApp::OnResize();

	DirectX::XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Engine::Math::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&ProjectionMatrix, P);
}

void FShapesApp::UpdateScene(float)
{
	// 구면 좌표계를 데카르트 좌표계로 변환
	float x = CameraRadius * sinf(CameraPhi) * cosf(CameraTheta);
	float z = CameraRadius * sinf(CameraPhi) * sinf(CameraTheta);
	float y = CameraRadius * cosf(CameraPhi);

	// view matrix 구성
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&ViewMatrix, view);
}

void FShapesApp::DrawScene()
{
	DeviceContext->ClearRenderTargetView(RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetInputLayout(InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetState(WireframeRasterizerState.Get());

	UINT stride = sizeof(FVertex);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// View * Projection 행렬 생성 World는 객체마다 바뀔것이기 때문에 미리 ViewProj 행렬만 계산해둠
	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX ViewProj = view * proj;

	// 파이프라인에 셰이더와 상수 버퍼 바인딩
	DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, PerObjectConstantBuffer.GetAddressOf());

	// 각 Shape 그리기
	DrawShapes(GridWorld, ViewProj, GridIndexCount, GridIndexOffset, GridVertexOffset);
	DrawShapes(BoxWorld, ViewProj, BoxIndexCount, BoxIndexOffset, BoxVertexOffset);
	DrawShapes(CenterSphereWorld, ViewProj, SphereIndexCount, SphereIndexOffset, SphereVertexOffset);
	for (int i = 0; i < 10; ++i)
	{
		DrawShapes(CylinderWorld[i], ViewProj, CylinderIndexCount, CylinderIndexOffset, CylinderVertexOffset);
	}
	for (int i = 0; i < 10; ++i)
	{
		DrawShapes(SphereWorld[i], ViewProj, SphereIndexCount, SphereIndexOffset, SphereVertexOffset);
	}


	ThrowIfFailed(SwapChain->Present(0, 0));
}

void FShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(AppWnd);
}

void FShapesApp::OnMouseUp(WPARAM, int, int)
{
	ReleaseCapture();
}

void FShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 1픽셀당 카메라를 0.25도 회전
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - LastMousePos.y));

		// 입력값을 바탕으로 카메라가 박스 주위를 공전하도록 각도를 갱신
		CameraTheta += dx;
		CameraPhi += dy;

		// 카메라가 극점에 도달하지 않도록 수직 회전 각도를 제한
		CameraPhi = std::clamp(CameraPhi, 0.1f, Engine::Math::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 1픽셀당 0.2 unit 만큼 카메라 거리 변화 (줌인, 줌아웃)
		float dx = 0.2f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - LastMousePos.y);

		// 입력값을 바탕으로 카메라 거리를 갱신
		CameraRadius += dx - dy;

		// 카메라 최대, 최소 거리 제한
		CameraRadius = std::clamp(CameraRadius, 3.0f, 200.0f);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}

void FShapesApp::DrawShapes(const XMFLOAT4X4& WorldMatrix, const XMMATRIX& ViewProj,  UINT IndexCount, UINT IndexOffset, INT VertexOffset)
{
	XMMATRIX World = XMLoadFloat4x4(&WorldMatrix);
	XMMATRIX WorldViewProj = World * ViewProj;

	FPerObjectConstants Constants{};
	XMStoreFloat4x4(&Constants.WorldViewProjection, XMMatrixTranspose(WorldViewProj));
	DeviceContext->UpdateSubresource(PerObjectConstantBuffer.Get(), 0, nullptr, &Constants, 0, 0);
	DeviceContext->DrawIndexed(IndexCount, IndexOffset, VertexOffset);
}

void FShapesApp::CreateGeometryBuffer()
{
	FGeometryGenerator::FMeshData Box;
	FGeometryGenerator::FMeshData Grid;
	FGeometryGenerator::FMeshData Sphere;
	FGeometryGenerator::FMeshData Cylinder;
	
	FGeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, Box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, Grid);
	geoGen.CreateSphere(0.5f, 20, 20, Sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, Cylinder);

	// 하나로 이어 붙인 정점 버퍼 안에서 각 도형의 정점 시작 위치를 저장
	BoxVertexOffset = 0;
	GridVertexOffset = Box.Vertices.size();
	SphereVertexOffset = GridVertexOffset + Grid.Vertices.size();
	CylinderVertexOffset = SphereVertexOffset + Sphere.Vertices.size();

	//각 도형을 그릴 때 사용할 인덱스 개수를 저장
	BoxIndexCount = Box.Indices.size();
	GridIndexCount = Grid.Indices.size();
	SphereIndexCount = Sphere.Indices.size();
	CylinderIndexCount = Cylinder.Indices.size();

	// 하나로 이어 붙인 인덱스 버퍼 안에서 각 도형의 인덱스 시작 위치를 저장
	BoxIndexOffset = 0;
	GridIndexOffset = BoxIndexCount;
	SphereIndexOffset = GridIndexOffset + GridIndexCount;
	CylinderIndexOffset = SphereIndexOffset + SphereIndexCount;

	UINT totalVertexCount =
		Box.Vertices.size() +
		Grid.Vertices.size() +
		Sphere.Vertices.size() +
		Cylinder.Vertices.size();

	UINT totalIndexCount =
		BoxIndexCount +
		GridIndexCount +
		SphereIndexCount +
		CylinderIndexCount;

	// 현재 렌더링에 필요한 정점 요소만 뽑아서,
	// 모든 mesh의 정점을 하나의 vertex buffer에 담는다.

	std::vector<FVertex> vertices(totalVertexCount);

	XMFLOAT4 black = ToFloat4(Colors::Black);

	UINT k = 0;
	for (size_t i = 0; i < Box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = Box.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < Grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = Grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < Sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = Sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for (size_t i = 0; i < Cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = Cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	D3D11_BUFFER_DESC VertexBufferDesc{};
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.ByteWidth = sizeof(FVertex) * totalVertexCount;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA VertexBufferSRD{};
	VertexBufferSRD.pSysMem = &vertices[0];
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, VertexBuffer.GetAddressOf()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), Box.Indices.begin(), Box.Indices.end());
	indices.insert(indices.end(), Grid.Indices.begin(), Grid.Indices.end());
	indices.insert(indices.end(), Sphere.Indices.begin(), Sphere.Indices.end());
	indices.insert(indices.end(), Cylinder.Indices.begin(), Cylinder.Indices.end());

	D3D11_BUFFER_DESC IndexBufferDesc{};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(UINT) * totalIndexCount;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA IndexBufferSRD{};
	IndexBufferSRD.pSysMem = &indices[0];
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, IndexBuffer.GetAddressOf()));
}

void FShapesApp::CreateShader()
{
	ComPtr<ID3DBlob> VertexShaderBlob;
	ComPtr<ID3DBlob> PixelShaderBlob;
	ComPtr<ID3DBlob> ErrorBlob;

	std::wstring ShaderPath = Framework::GetShaderPathString(__FILE__, L"Color.hlsl");

	ThrowIfFailed(D3DCompileFromFile(
		ShaderPath.c_str(),
		nullptr,
		nullptr,
		"MainVS",
		"vs_5_0",
		0,
		0,
		VertexShaderBlob.GetAddressOf(),
		ErrorBlob.GetAddressOf()
	));

	ThrowIfFailed(Device->CreateVertexShader(
		VertexShaderBlob->GetBufferPointer(),
		VertexShaderBlob->GetBufferSize(),
		nullptr,
		VertexShader.GetAddressOf()
	));

	ThrowIfFailed(D3DCompileFromFile(
		ShaderPath.c_str(),
		nullptr,
		nullptr,
		"MainPS",
		"ps_5_0",
		0,
		0,
		PixelShaderBlob.GetAddressOf(),
		ErrorBlob.ReleaseAndGetAddressOf()
	));

	ThrowIfFailed(Device->CreatePixelShader(
		PixelShaderBlob->GetBufferPointer(),
		PixelShaderBlob->GetBufferSize(),
		nullptr,
		PixelShader.GetAddressOf()
	));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ThrowIfFailed(Device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		VertexShaderBlob->GetBufferPointer(),
		VertexShaderBlob->GetBufferSize(),
		InputLayout.GetAddressOf()
	));
}

void FShapesApp::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC ConstantBufferDesc = {};
	ConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ConstantBufferDesc.ByteWidth = sizeof(FPerObjectConstants);
	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDesc.CPUAccessFlags = 0;
	ConstantBufferDesc.MiscFlags = 0;
	ConstantBufferDesc.StructureByteStride = 0;

	ThrowIfFailed(Device->CreateBuffer(&ConstantBufferDesc, nullptr, PerObjectConstantBuffer.GetAddressOf()));
}


std::unique_ptr<FD3DApp> GetApplication(HINSTANCE hInstance)
{
	return std::make_unique<FShapesApp>(hInstance);
}

