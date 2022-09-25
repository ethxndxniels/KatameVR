cbuffer DirectionalLight : register(b2)
{
    float4 lightDir;
};

struct PSVertex
{
    float4 Pos : SV_POSITION;
    float3 Normal : Normal;
};

float4 main( PSVertex input ) : SV_Target
{
    return float4(lightDir.xyz, 1);
}