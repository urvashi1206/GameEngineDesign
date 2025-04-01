#include "ShaderStructs.hlsli"
#include "ShaderFunctions.hlsli"

#define MAX_LIGHTS 64

cbuffer DataFromCPU : register(b0) // Take the data from memory register b0 ("buffer 0")
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
	float3 cameraLocation;

	int lightCount;
	Light lights[MAX_LIGHTS];
}

// Set of options for sampling
SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// Define textures
Texture2D AlbedoTexture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.shadowMapPosition.xyz /= input.shadowMapPosition.w; // Manual perspective divide
    
    float2 shadowUV = input.shadowMapPosition.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y;

    float distanceToLight = input.shadowMapPosition.z;
    // Get the amount of shadowing by comparing distance to light and the sampled shadow map value
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distanceToLight).r;

    // Apply UV transformations based on material settings
    input.uv *= uvScale;
    input.uv += uvOffset;

    // Sample textures
    float3 textureColor = pow(AlbedoTexture.Sample(BasicSampler, input.uv).rgb * colorTint.rgb, 2.2f); // Reverse gamma correction of surface texture - final color is re-corrected
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    float3 specularColor = lerp(F0_NON_METAL, textureColor, metalness); // Specular is somewhere between a constant and the albedo color, depending on metalness
    
    float3 unpackedNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1);

    // Calculate normal, tangent and bitangent for normal mapping
    // Orthonormalize normal and tangent using Gram-Schmidt Process
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - N * dot(N, input.tangent));
    float3 B = cross(T, N);
    float3x3 rotationMatrix = float3x3(T, B, N); // Rotation matrix that transforms tangent space to world space

    // Transform normal value from normal map to world space and update the input parameter
	//input.normal = normalize(mul(unpackedNormal, rotationMatrix));

    // Add final color results from all lights
    float3 totalColor = 0;
	for(int i = 0; i < lightCount; i++)
    {
        switch(lights[i].LightType)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalColor += DirectionalLightPBR(lights[i], input.normal, input.worldPosition, cameraLocation, roughness, metalness, textureColor, specularColor);
                break;
            case LIGHT_TYPE_POINT:
                totalColor += PointLightPBR(lights[i], input.normal, input.worldPosition, cameraLocation, roughness, metalness, textureColor, specularColor);
                break;
            default:
                break;
        }
        
        // Assume the first light is the shadow-casting light; scale by shadow amount retrieved from shadow sampler
        if(i == 0)
            totalColor *= shadowAmount;
    }
    
    //return float4(input.normal, 1);
    //return float4(input.uv, 1, 1);
    //return AlbedoTexture.Sample(BasicSampler, input.uv);
    return float4(pow(totalColor, 1.0f / 2.2f), 1); // Gamma correct the final result
}