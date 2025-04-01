#ifndef __SHADER_STRUCTS__
#define __SHADER_STRUCTS__

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#define MAX_SPECULAR_EXPONENT   256.0f

struct Light
{
    int LightType;
    float3 Direction;
    float Range;
    float3 Location;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;
    float3 tangent          : TANGENT;
	float2 uv				: TEXCOORD;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	float4 screenPosition   	: SV_POSITION;
	float3 normal   			: NORMAL;
    float3 tangent              : TANGENT;
	float2 uv   				: TEXCOORD;
	float3 worldPosition    	: POSITION;
    float4 shadowMapPosition    : SHADOW_POSITION;
};

#endif