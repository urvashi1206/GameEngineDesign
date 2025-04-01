#include "ShaderStructs.hlsli"

cbuffer DataFromCPU : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexToPixel_Sky
{
	float4 position				: SV_POSITION;
	float3 sampleDirection		: DIRECTION;
};

VertexToPixel_Sky main(VertexShaderInput input)
{
	VertexToPixel_Sky output;

	matrix viewNoTranslation = viewMatrix;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	output.position = mul(projectionMatrix, mul(viewNoTranslation, float4(input.localPosition, 1)));
	output.position.z = output.position.w; // Ensure depth is 1.0 (z/w = 1.0)

	output.sampleDirection = input.localPosition;

	return output;
}