cbuffer TransformBuffer : register(b0) 
{
	float4x4 World;
	float4x4 ViewProj;
};

struct vsIn 
{
	float3 a_Position  : Position;
	float3 a_Color : Color;
};

struct psIn 
{
	float4 a_Position   : SV_POSITION;
	float3 a_Color : Color;
};

psIn main( vsIn input ) 
{
	psIn output;
	output.a_Position = mul( float4(input.a_Position.xyz, 1), World );
	output.a_Position = mul( output.a_Position, ViewProj );
	output.a_Color = input.a_Color;
	return output;
}