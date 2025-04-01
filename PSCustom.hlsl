#include "ShaderStructs.hlsli"

cbuffer DataFromCPU : register(b0)
{
    float4 colorTint;
    float totalTime;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    // A varying pixel size creates a "pixelation" effect when combined with the given noise algorithm
    // The UV term makes the pixelation change propogate throughout the surface of the mesh
    int pixelSize = 100 / totalTime - input.uv.y;
    
    // The random noise value, determined by the screen position divided by pixel size (so pixels in the same area have the same noise value)
    float random = frac(sin(dot((uint2) input.screenPosition / pixelSize, float2(12.9898, 78.233))) * 43758.5453123);
    
    // Display the random value in red
    return float4(random, 0, 0, 1);
}