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

class FSkullApp : public FD3DApp
{
public:
	FSkullApp(HINSTANCE hInstance);
	~FSkullApp() override = default;

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
	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;
	ComPtr<ID3D11Buffer> PerObjectConstantBuffer;

	ComPtr<ID3D11VertexShader> VertexShader;
	ComPtr<ID3D11PixelShader> PixelShader;
	ComPtr<ID3D11InputLayout> InputLayout;

	ComPtr<ID3D11RasterizerState> WireframeRasterizerState;

	XMFLOAT4X4 SkullWorld;
	XMFLOAT4X4 ViewMatrix;
	XMFLOAT4X4 ProjectionMatrix;

	UINT SkullIndexCount = 0;

	// 카메라의 수평 방향 각도
	float CameraTheta;
	// 카메라의 수직 방향 각도
	float CameraPhi;
	// 원점에서 카메라까지의 거리
	float CameraRadius;

	POINT LastMousePos;
};

FSkullApp::FSkullApp(HINSTANCE hInstance)
	: FD3DApp(hInstance),
	CameraTheta(1.5f * Engine::Math::Pi),
	CameraPhi(0.1f * Engine::Math::Pi),
	CameraRadius(20.0f),
	LastMousePos()
{
	MainWndCaption = L"Skull Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&ViewMatrix, I);
	XMStoreFloat4x4(&ProjectionMatrix, I);

	XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&SkullWorld, T);
}

bool FSkullApp::Init()
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

void FSkullApp::OnResize()
{
	FD3DApp::OnResize();

	DirectX::XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Engine::Math::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&ProjectionMatrix, P);
}

void FSkullApp::UpdateScene(float)
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

void FSkullApp::DrawScene()
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

	// Set constatns
	XMMATRIX world = XMLoadFloat4x4(&SkullWorld);
	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX WorldViewProj = world * view * proj;

	// 파이프라인에 셰이더와 상수 버퍼 바인딩
	DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, PerObjectConstantBuffer.GetAddressOf());
	
	// Constant Buffer 갱신
	FPerObjectConstants constants{};
	DirectX::XMStoreFloat4x4(&constants.WorldViewProjection, DirectX::XMMatrixTranspose(WorldViewProj));
	DeviceContext->UpdateSubresource(PerObjectConstantBuffer.Get(), 0, nullptr, &constants, 0, 0);

	// Draw the grid
	DeviceContext->DrawIndexed(SkullIndexCount, 0, 0);


	ThrowIfFailed(SwapChain->Present(0, 0));
}

void FSkullApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	SetCapture(AppWnd);
}

void FSkullApp::OnMouseUp(WPARAM, int, int)
{
	ReleaseCapture();
}

void FSkullApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		// 1픽셀당 0.05 unit 만큼 카메라 거리 변화 (줌인, 줌아웃)
		float dx = 0.05f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - LastMousePos.y);

		// 입력값을 바탕으로 카메라 거리를 갱신
		CameraRadius += dx - dy;

		// 카메라 최대, 최소 거리 제한
		CameraRadius = std::clamp(CameraRadius, 5.0f, 50.0f);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}


void FSkullApp::CreateGeometryBuffer()
{
	std::ifstream fin(Framework::GetModelPath(__FILE__, L"skull.txt"));

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<FVertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;

		vertices[i].Color = black;

		// Normal not used in this demo.
		fin >> nx >> ny >> nz;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	SkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(SkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC VertexBufferDesc{};
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.ByteWidth = sizeof(FVertex) * vcount;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA VertexBufferSRD{};
	VertexBufferSRD.pSysMem = &vertices[0];
	ThrowIfFailed(Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, VertexBuffer.GetAddressOf()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC IndexBufferDesc{};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(UINT) * SkullIndexCount;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA IndexBufferSRD{};
	IndexBufferSRD.pSysMem = &indices[0];
	ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, IndexBuffer.GetAddressOf()));
}

void FSkullApp::CreateShader()
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

void FSkullApp::CreateConstantBuffer()
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
	return std::make_unique<FSkullApp>(hInstance);
}

