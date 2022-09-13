struct PSVertex
{
    float4 Pos : SV_POSITION;
    float3 Color : Color;
};

struct Vertex 
{
    float3 Pos : Position;
    float3 Color : Color;
};

cbuffer ModelConstantBuffer : register(b0) 
{
    float4x4 Model;
};

cbuffer ViewProjectionConstantBuffer : register(b1) 
{
    float4x4 ViewProjection;
};

PSVertex main( Vertex input ) {
    PSVertex output;
    output.Pos = mul( mul( float4(input.Pos, 1), Model ), ViewProjection );
    output.Color = input.Color;
    return output;
}
