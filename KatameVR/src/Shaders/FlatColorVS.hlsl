cbuffer ModelConstantBuffer : register(b0)
{
	float4x4 Model;
};

cbuffer ViewProjectionConstantBuffer : register(b1)
{
	float4x4 ViewProjection;
};

struct vsIn
{
	float3 a_Position  : Position;
	float3 a_Color : Color;
};

struct psIn
{
	float4 a_Position  : SV_POSITION;
	float3 a_Color : Color;
};

psIn main( vsIn input )
{
	psIn output;
	output.a_Position = mul( mul( float4(input.a_Position, 1), Model ), ViewProjection );
	output.a_Color = input.a_Color;
	return output;
}