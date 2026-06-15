// 상수 버퍼
cbuffer PerObjectConstants : register(b0)
{
    float4x4 WorldViewProjection;
};

struct FVertexInput
{
	float3 PositionLocal : POSITION;
    float4 Color : COLOR;
};

struct FVertexOutput
{
	float4 PositionClip  : SV_POSITION;
    float4 Color : COLOR;
};

// 버텍스 셰이더
FVertexOutput MainVS(FVertexInput Input)
{
    FVertexOutput Output;
	
	// 동차 클립 좌표로 변환
    Output.PositionClip = mul(float4(Input.PositionLocal, 1.0f), WorldViewProjection);
	
	// Color은 그대로
    Output.Color = Input.Color;
    
    return Output;
}

// 픽셀 셰이더
float4 MainPS(FVertexOutput Input) : SV_Target
{
    return Input.Color;
}
