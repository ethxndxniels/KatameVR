cbuffer DirectionalLight : register(b2)
{
	float4 lightDir;
};

cbuffer pointLight : register(b3)
{
	float3 pointPosition;
	float3 color;
	float radius;
	float padding;
};

Texture2D tex;
Texture2D spec : register(t1);

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
	// Ambient
	float3 ambient = { 0.05f, 0.05f, 0.05f };

	// Directional Light
	float3 directionalDiffuse = dot(input.a_Normal, -lightDir.xyz).rrr;

	// Point Light
	float3 pointToFrag = pointPosition - input.a_ModelPosition.xyz;
	float3 pointDiffuse = max(0, dot(normalize(pointToFrag), input.a_Normal).rrr);
	float pointLightAtt = (1 - (length(pointToFrag) / radius));
	pointLightAtt *= pointLightAtt;
	pointDiffuse *= pointLightAtt;

	//float3 pointLightSpec = pow(max(0.0f, dot(input.a_Normal, camToFrag)), 128);

	// Final Color
	return float4(tex.Sample(splr, input.a_TexCoord).rgb * saturate( pointDiffuse + ambient), 1);
}