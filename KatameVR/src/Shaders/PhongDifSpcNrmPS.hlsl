cbuffer DirectionalLight : register(b2)
{
	float4 lightDir;
};

Texture2D tex;
SamplerState splr;

struct psIn
{
	float4 a_Position : SV_POSITION;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
};

float4 main( psIn input ) : SV_Target
{
	float3 diffuse = dot(normalize(float4(input.a_Normal, 0.0f)), -lightDir).rrr;
	return float4(diffuse * tex.Sample(splr, input.a_TexCoord).rgb, 1); 
}