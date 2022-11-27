struct PSVertex
{
    float4 Pos : SV_POSITION;
    float3 a_Color : Color;
};

float4 main( PSVertex input ) : SV_Target
{
    return float4( input.a_Color, 1);
}