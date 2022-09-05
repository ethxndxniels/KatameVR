struct PSVertex 
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
};

float4 MainPS( PSVertex input ) : SV_TARGET
{
    return float4(input.Color, 1);
}