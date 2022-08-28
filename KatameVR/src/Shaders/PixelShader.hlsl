cbuffer DirectionalLight : register(b1)
{
	float4 lightDir;
};

struct psIn 
{
	float4 a_Position   : SV_POSITION;
	float3 a_Normal : Normal;
};

float4 main( psIn input ) : SV_TARGET
{
	float diffuse = saturate( dot( float4(input.a_Normal, 0), -lightDir ) );
	return float4(diffuse.rrr, 1);
}