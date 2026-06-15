#include "D3DApp.h"
#include "DebugLog.h"
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

class FBoxApp : public FD3DApp
{
public:
	FBoxApp(HINSTANCE hInstance);
	~FBoxApp() override = default;

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void CreateGeometryBuffer();
	void CreateShader();
	void CreateConstantBuffer();

private:
	ComPtr<ID3D11Buffer> BoxVertexBuffer;
	ComPtr<ID3D11Buffer> BoxIndexBuffer;
	ComPtr<ID3D11Buffer> PerObjectConstantBuffer;

	ComPtr<ID3D11VertexShader> VertexShader;
	ComPtr<ID3D11PixelShader> PixelShader;
	ComPtr<ID3D11InputLayout> InputLayout;

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

FBoxApp::FBoxApp(HINSTANCE hInstance)
	: FD3DApp(hInstance),
	CameraTheta(1.5f * Engine::Math::Pi),
	CameraPhi(0.25f * Engine::Math::Pi),
	CameraRadius(5.0f),
	LastMousePos()
{
	MainWndCaption = L"Box Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&WorldMatrix, I);
	XMStoreFloat4x4(&ViewMatrix, I);
	XMStoreFloat4x4(&ProjectionMatrix, I);
}

bool FBoxApp::Init()
{
	if (!FD3DApp::Init())
		return false;

	CreateGeometryBuffer();
	CreateShader();
	CreateConstantBuffer();
	

	return true;
}

void FBoxApp::OnResize()
{
	FD3DApp::OnResize();

	// 창 크기가 변경되었으므로, 새 종횡비에 맞춰 투영 행렬을 다시 계산한다.
	DirectX::XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Engine::Math::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&ProjectionMatrix, P);
}

void FBoxApp::UpdateScene(float)
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

void FBoxApp::DrawScene()
{
	DeviceContext->ClearRenderTargetView(RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetInputLayout(InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(FVertex);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, BoxVertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(BoxIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

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

	// 36 indices for the box.
	DeviceContext->DrawIndexed(36, 0, 0);

	ThrowIfFailed(SwapChain->Present(0, 0));
}

void FBoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(AppWnd);
}

void FBoxApp::OnMouseUp(WPARAM, int, int)
{
	ReleaseCapture();
}

void FBoxApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		// 1픽셀당 0.005 unit 만큼 카메라 거리 변화 (줌인, 줌아웃)
		float dx = 0.005f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - LastMousePos.y);

		// 입력값을 바탕으로 카메라 거리를 갱신
		CameraRadius += dx - dy;

		// 카메라 최대, 최소 거리 제한
		CameraRadius = std::clamp(CameraRadius, 3.0f, 15.0f);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}

void FBoxApp::CreateGeometryBuffer()
{
	// vertex buffer 생성
	FVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), ToFloat4(Colors::White)},
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), ToFloat4(Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), ToFloat4(Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), ToFloat4(Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), ToFloat4(Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), ToFloat4(Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), ToFloat4(Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), ToFloat4(Colors::Magenta) }
	};

	D3D11_BUFFER_DESC VertexBufferDesc{};
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.ByteWidth = sizeof(FVertex) * 8;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA VertexBufferSRD{};
	VertexBufferSRD.pSysMem = vertices;
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, BoxVertexBuffer.GetAddressOf()));


	// 인덱스 버퍼 생성

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC IndexBufferDesc{};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(UINT) * 36;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA IndexBufferSRD{};
	IndexBufferSRD.pSysMem = indices;
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, BoxIndexBuffer.GetAddressOf()));
}

void FBoxApp::CreateShader()
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

void FBoxApp::CreateConstantBuffer()
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
	return std::make_unique<FBoxApp>(hInstance);
}
