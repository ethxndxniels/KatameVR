cbuffer DirectionalLight : register(b2)
{
	float4 lightDir;
};

cbuffer pointLight : register(b3)
{
	float3 pointPosition;
	float3 color;
};

Texture2D tex;
Texture2D nmap : register(t2);

SamplerState splr;

struct psIn
{
	float4 a_ModelPosition  : Position;
	float3 a_Normal : Normal;
	float2 a_TexCoord : Texcoord;
	float4 a_Position  : SV_POSITION;
};

float4 main(psIn input) : SV_Target
{
	// TODO
	//  - Add useNormalMap conditional
	// Normal
	float3 sampledNormal = nmap.Sample(splr, input.a_TexCoord).xyz;

	// Ambient
	float3 ambient = { 0.1f, 0.1f, 0.1f };

	// Directional Light
	float3 directionalDiffuse = dot(sampledNormal, -lightDir.xyz).rrr;

	// Point Light
	float3 pointToFrag = pointPosition - input.a_ModelPosition.xyz;
	float3 pointDiffuse = dot(normalize(pointToFrag), sampledNormal).rrr;
	float pointLightAtt = saturate((1 - (length(pointToFrag) / 100.0f)));
	pointLightAtt *= pointLightAtt;
	pointDiffuse *= pointLightAtt;

	// Final Color
	return float4(tex.Sample(splr, input.a_TexCoord).rgb * saturate( pointDiffuse + ambient), 1);
}