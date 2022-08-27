struct psIn {
	float4 pos   : Position;
	float3 color : COLOR0;
};

float4 main( psIn input ) : SV_TARGET{
	return float4(input.color, 1);
}