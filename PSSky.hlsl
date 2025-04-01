struct VertexToPixel_Sky
{
	float4 position				: SV_POSITION;
	float3 sampleDirection		: DIRECTION;
};

SamplerState SkySampler : register(s0);
TextureCube SkyCubeMap : register(t0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
	// Just sample the cubemap with the given direction
    return SkyCubeMap.Sample(SkySampler, input.sampleDirection);
}