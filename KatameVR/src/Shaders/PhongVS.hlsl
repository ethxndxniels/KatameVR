cbuffer TransformBuffer : register(b0) 
{
	float4x4 world;
	float4x4 viewproj;
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
	float4 a_Position  : SV_POSITION;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
};

psIn main( vsIn input )
{
	psIn output;
	output.a_Position = mul( float4(input.a_Position.xyz, 1), world );
	output.a_Position = mul( output.a_Position, viewproj );
	output.a_Normal = normalize( mul( float4(input.a_Normal, 0), world ).xyz );
	output.a_TexCoord = input.a_TexCoord;
	return output;
}