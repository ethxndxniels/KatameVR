cbuffer DirectionalLight : register(b1)
{
	float4 lightDir;
};

struct psIn 
{
	float4 a_Position   : SV_POSITION;
	float3 a_Color : Color;
};

float4 main( psIn input ) : SV_TARGET
{
	return float4(input.a_Color, 1);
}