struct psIn
{
	float4 a_Position : SV_POSITION;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
};

float4 main( psIn input ) : SV_Target
{
	return float4( 1.0f, 1.0f, 1.0f, 1.0f );
}