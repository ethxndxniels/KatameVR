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
Texture2D nmap : register(t2);

SamplerState splr;

struct psIn
{
	float4 a_ModelPosition  : Position;
	float3 a_Normal : Normal;
	float3 a_Tangent  : Tangent;
	float3 a_Bitangent : Bitangent;
	float2 a_TexCoord : Texcoord;
	float4 a_Position  : SV_POSITION;
};

float4 main(psIn input) : SV_Target
{
	// Normal
	const float3x3 tanToTarget = float3x3(input.a_Tangent, input.a_Bitangent, input.a_Normal);
	// sample and unpack the normal from texture into target space   
	const float3 normalSample = nmap.Sample(splr, input.a_TexCoord).xyz;
	const float3 tanNormal = normalSample * 2.0f - 1.0f;
	// bring normal from tanspace into target space
	float3 calcedNorm = normalize(mul(tanNormal, tanToTarget));

	// Ambient
	float3 ambient = { 0.05f, 0.05f, 0.05f };

	// Directional Light
	float3 directionalDiffuse = dot(calcedNorm, -lightDir.xyz).rrr;

	// Point Light
	float3 pointToFrag = pointPosition - input.a_ModelPosition.xyz;
	float3 pointDiffuse = max(0, dot(normalize(pointToFrag), calcedNorm).rrr);
	float pointLightAtt = (1 - (length(pointToFrag) / radius));
	pointLightAtt *= pointLightAtt;
	pointDiffuse *= pointLightAtt;

	// Final Color
	return float4(tex.Sample(splr, input.a_TexCoord).rgb * saturate( pointDiffuse + ambient), 1);
}