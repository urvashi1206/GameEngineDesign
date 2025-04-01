#include "ShaderStructs.hlsli"

cbuffer DataFromCPU : register(b0) // Take the data from memory register b0 ("buffer 0")
{
	matrix worldMatrix;
	matrix worldInvTranspose;
	matrix viewMatrix;
	matrix projMatrix;
	
    matrix lightView;
    matrix lightProj;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	
	matrix wvp = mul(projMatrix, mul(viewMatrix, worldMatrix));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	
	// Rotate normal and tangent to match object's world transform
    output.normal = mul((float3x3) worldInvTranspose, input.normal);
	output.tangent = mul((float3x3) worldMatrix, input.tangent);
	
    output.uv = input.uv;

	output.worldPosition = mul(worldMatrix, float4(input.localPosition, 1)).xyz;

    matrix shadowWVP = mul(lightProj, mul(lightView, worldMatrix));
    output.shadowMapPosition = mul(shadowWVP, float4(input.localPosition, 1.0f));

	return output;
}