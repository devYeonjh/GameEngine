#include "D3DApp.h"
#include "DebugLog.h"
#include "GeometryGenerator.h"
#include "Math/MathUtility.h"
#include "DemoPathUtil.h"
#include "Waves.h"

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

class FWavesApp : public FD3DApp
{
public:
	FWavesApp(HINSTANCE hInstance);
	~FWavesApp() override = default;

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z)const;
	void CreateLandGeometryBuffers();
	void CreateWavesGeometryBuffers();
	void CreateShader();
	void CreateConstantBuffer();

private:
	ComPtr<ID3D11Buffer> LandVertexBuffer;
	ComPtr<ID3D11Buffer> LandIndexBuffer;
	ComPtr<ID3D11Buffer> WavesVertexBuffer;
	ComPtr<ID3D11Buffer> WavesIndexBuffer;
	ComPtr<ID3D11Buffer> PerObjectConstantBuffer;

	ComPtr<ID3D11VertexShader> VertexShader;
	ComPtr<ID3D11PixelShader> PixelShader;
	ComPtr<ID3D11InputLayout> InputLayout;

	ComPtr<ID3D11RasterizerState> WireframeRasterizerState;

	XMFLOAT4X4 GridWorld;
	XMFLOAT4X4 WavesWorld;

	UINT GridIndexCount = 0;

	FWaves Waves;

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

FWavesApp::FWavesApp(HINSTANCE hInstance)
	: FD3DApp(hInstance),
	CameraTheta(1.5f * Engine::Math::Pi),
	CameraPhi(0.1f * Engine::Math::Pi),
	CameraRadius(200.0f),
	LastMousePos()
{
	MainWndCaption = L"Waves Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&GridWorld, I);
	XMStoreFloat4x4(&WavesWorld, I);
	XMStoreFloat4x4(&ViewMatrix, I);
	XMStoreFloat4x4(&ProjectionMatrix, I);

}

bool FWavesApp::Init()
{
	if (!FD3DApp::Init())
		return false;

	Waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	CreateLandGeometryBuffers();
	CreateWavesGeometryBuffers();
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

void FWavesApp::OnResize()
{
	FD3DApp::OnResize();

	DirectX::XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Engine::Math::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&ProjectionMatrix, P);
}

void FWavesApp::UpdateScene(float dt)
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

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if ((Timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		Engine::Math::FRandomStream RandomStream{};
		float r = RandomStream.RandF(1.0f, 2.0f);

		Waves.Disturb(i, j, r);
	}

	Waves.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	ThrowIfFailed(DeviceContext->Map(WavesVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	FVertex* v = reinterpret_cast<FVertex*>(mappedData.pData);
	for (UINT i = 0; i < Waves.GetVertexCount(); ++i)
	{
		v[i].Position = Waves[i];
		v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	DeviceContext->Unmap(WavesVertexBuffer.Get(), 0);
}

void FWavesApp::DrawScene()
{
	DeviceContext->ClearRenderTargetView(RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetInputLayout(InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	UINT stride = sizeof(FVertex);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);

	// 파이프라인에 셰이더와 상수 버퍼 바인딩
	DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, PerObjectConstantBuffer.GetAddressOf());

	// Draw the land
	DeviceContext->IASetVertexBuffers(0, 1, LandVertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(LandIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&GridWorld);
	XMMATRIX worldViewProj = world * view * proj;
	FPerObjectConstants constants{};
	DirectX::XMStoreFloat4x4(&constants.WorldViewProjection, DirectX::XMMatrixTranspose(worldViewProj));
	DeviceContext->UpdateSubresource(PerObjectConstantBuffer.Get(), 0, nullptr, &constants, 0, 0);
	DeviceContext->DrawIndexed(GridIndexCount, 0, 0);

	// Draw the waves
	DeviceContext->RSSetState(WireframeRasterizerState.Get());

	DeviceContext->IASetVertexBuffers(0, 1, WavesVertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(WavesIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	world = XMLoadFloat4x4(&WavesWorld);
	worldViewProj = world * view * proj;
	DirectX::XMStoreFloat4x4(&constants.WorldViewProjection, DirectX::XMMatrixTranspose(worldViewProj));
	DeviceContext->UpdateSubresource(PerObjectConstantBuffer.Get(), 0, nullptr, &constants, 0, 0);
	DeviceContext->DrawIndexed(3 * Waves.GetTriangleCount(), 0, 0);

	DeviceContext->RSSetState(nullptr);

	ThrowIfFailed(SwapChain->Present(0, 0));
}

void FWavesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(AppWnd);
}

void FWavesApp::OnMouseUp(WPARAM, int, int)
{
	ReleaseCapture();
}

void FWavesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		CameraRadius = std::clamp(CameraRadius, 50.0f, 500.0f);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}

float FWavesApp::GetHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void FWavesApp::CreateLandGeometryBuffers()
{
	FGeometryGenerator::FMeshData Grid;

	FGeometryGenerator GeoGen;

	GeoGen.CreateGrid(160.0f, 160.0f, 50, 50, Grid);

	GridIndexCount = Grid.Indices.size();

	// 필요한 vertex 요소(Posotion)만 추출하고, 각 vertex에 height function을 적용한다.
	// 추가로 vertex의 height에 따라 색을 지정해서 모래사장, 풀이 있는 낮은 언덕,
	// 바위/흙 지대, 눈 덮인 산봉우리처럼 보이게 만든다.

	std::vector<FVertex> vertices(Grid.Vertices.size());
	for (size_t i = 0; i < Grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = Grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Position = p;

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			vertices[i].Color = ToFloat4(Colors::SandyBrown);
		}
		else if (p.y < 5.0f)
		{
			vertices[i].Color = ToFloat4(Colors::YellowGreen);
		}
		else if (p.y < 12.0f)
		{
			vertices[i].Color = ToFloat4(Colors::ForestGreen);
		}
		else if (p.y < 20.0f)
		{
			vertices[i].Color = ToFloat4(Colors::SaddleBrown);
		}
		else
		{
			vertices[i].Color = ToFloat4(Colors::Snow);
		}
	}

	D3D11_BUFFER_DESC VertexBufferDesc{};
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.ByteWidth = sizeof(FVertex) * Grid.Vertices.size();
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA VertexBufferSRD{};
	VertexBufferSRD.pSysMem = &vertices[0];
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, LandVertexBuffer.GetAddressOf()));

	//
	// 모든 mesh의 index들을 하나의 index buffer에 묶어 넣는다.
	//

	D3D11_BUFFER_DESC IndexBufferDesc{};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(UINT) * GridIndexCount;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA IndexBufferSRD{};
	IndexBufferSRD.pSysMem = &Grid.Indices[0];
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, LandIndexBuffer.GetAddressOf()));
}

void FWavesApp::CreateWavesGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC VertexBufferDesc{};
	VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VertexBufferDesc.ByteWidth = sizeof(FVertex) * Waves.GetVertexCount();
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VertexBufferDesc.MiscFlags = 0;
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, 0, WavesVertexBuffer.GetAddressOf()));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3 * Waves.GetTriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = Waves.GetRowCount();
	UINT n = Waves.GetColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC IndexBufferDesc{};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA IndexBufferSRD{};
	IndexBufferSRD.pSysMem = &indices[0];
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, WavesIndexBuffer.GetAddressOf()));
}

void FWavesApp::CreateShader()
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

void FWavesApp::CreateConstantBuffer()
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
	return std::make_unique<FWavesApp>(hInstance);
}

