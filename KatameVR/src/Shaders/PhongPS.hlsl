Texture2D tex;
SamplerState splr;

struct psIn
{
	float4 a_Position  : SV_POSITION;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
};

float4 main( psIn input ) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);//float4(dot( input.a_Normal, float3(0,1,0) ) * tex.Sample(splr, input.a_TexCoord).rgb, 1.0f);
}