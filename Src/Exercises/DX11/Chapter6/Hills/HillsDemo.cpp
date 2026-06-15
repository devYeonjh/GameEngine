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

class FHillsApp : public FD3DApp
{
public:
	FHillsApp(HINSTANCE hInstance);
	~FHillsApp() override = default;

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	float GetHeight(float x, float z) const;
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
	
	UINT GridIndexCount;

	XMFLOAT4X4 WorldMatrix;
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

FHillsApp::FHillsApp(HINSTANCE hInstance)
	: FD3DApp(hInstance),
	GridIndexCount(0),
	CameraTheta(1.5f * Engine::Math::Pi),
	CameraPhi(0.1f * Engine::Math::Pi),
	CameraRadius(200.0f),
	LastMousePos()
{
	MainWndCaption = L"Hills Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&WorldMatrix, I);
	XMStoreFloat4x4(&ViewMatrix, I);
	XMStoreFloat4x4(&ProjectionMatrix, I);
}

bool FHillsApp::Init()
{
	if (!FD3DApp::Init())
		return false;

	CreateGeometryBuffer();
	CreateShader();
	CreateConstantBuffer();
	

	return true;
}

void FHillsApp::OnResize()
{
	FD3DApp::OnResize();

	DirectX::XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Engine::Math::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&ProjectionMatrix, P);
}

void FHillsApp::UpdateScene(float)
{
	// 구면 좌표계를 데카르트 좌표계로 변환
	float x = CameraRadius * sinf(CameraPhi) * cosf(CameraTheta);
	float z = CameraRadius * sinf(CameraPhi) * sinf(CameraTheta);
	float y = CameraRadius * cosf(CameraPhi);

	// view matrix 구성
	XMVECTOR pos	= XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target	= XMVectorZero();
	XMVECTOR up		= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&ViewMatrix, view);
}

void FHillsApp::DrawScene()
{
	DeviceContext->ClearRenderTargetView(RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetInputLayout(InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(FVertex);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&WorldMatrix);
	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX worldViewProj = world * view * proj;

	// 파이프라인에 셰이더와 상수 버퍼 바인딩
	DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, PerObjectConstantBuffer.GetAddressOf());

	// Constant Buffer 갱신
	FPerObjectConstants constants{};
	DirectX::XMStoreFloat4x4(&constants.WorldViewProjection, DirectX::XMMatrixTranspose(worldViewProj));
	DeviceContext->UpdateSubresource(PerObjectConstantBuffer.Get(), 0, nullptr, &constants, 0, 0);

	// Draw the grid
	DeviceContext->DrawIndexed(GridIndexCount, 0, 0);

	ThrowIfFailed(SwapChain->Present(0, 0));
}

void FHillsApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(AppWnd);
}

void FHillsApp::OnMouseUp(WPARAM, int, int)
{
	ReleaseCapture();
}

void FHillsApp::OnMouseMove(WPARAM btnState, int x, int y)
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

float FHillsApp::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void FHillsApp::CreateGeometryBuffer()
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
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, VertexBuffer.GetAddressOf()));

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
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, IndexBuffer.GetAddressOf()));
}

void FHillsApp::CreateShader()
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

void FHillsApp::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC ConstantBufferDesc{};
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
	return std::make_unique<FHillsApp>(hInstance);
}

