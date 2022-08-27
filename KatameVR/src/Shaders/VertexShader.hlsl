cbuffer TransformBuffer : register(b0) {
	float4x4 world;
	float4x4 viewproj;
};
struct vsIn {
	float3 pos  : Position;
	float3 norm : Normal;
};
struct psIn {
	float4 pos   : Position;
	float3 color : Color;
};
psIn main( vsIn input ) {
	psIn output;
	output.pos = mul( float4(input.pos.xyz, 1), world );
	output.pos = mul( output.pos, viewproj );
	float3 normal = normalize( mul( float4(input.norm, 0), world ).xyz );
	output.color = saturate( dot( normal, float3(0, 1, 0) ) ).xxx;
	return output;
}