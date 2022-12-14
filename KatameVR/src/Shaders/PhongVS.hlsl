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
	float3 a_Normal : Normal;
	float3 a_Tangent  : Tangent;
	float3 a_Bitangent : Bitangent;
	float2 a_TexCoord  : Texcoord;
};

struct psIn 
{
	float4 a_ModelPosition  : Position;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
	float4 a_Position  : SV_POSITION;
};

psIn main( vsIn input )
{
	psIn output;
	output.a_Position = mul( mul( float4(input.a_Position, 1), Model ), ViewProjection );
	output.a_ModelPosition = mul(float4(input.a_Position, 1), Model);
	output.a_Normal = normalize( mul( input.a_Normal, (float3x3)Model ).xyz );
	output.a_TexCoord = input.a_TexCoord;
	return output;
}