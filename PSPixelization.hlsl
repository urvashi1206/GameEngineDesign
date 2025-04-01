cbuffer ExternalData : register(b0)
{
    int pixelSize;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);

SamplerState Sampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    /* Pixelization algorithm from 
    https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html */
    
    float2 textureSize;
    Pixels.GetDimensions(textureSize.x, textureSize.y);
    
    float x = uint(input.position.x) % pixelSize;
    float y = uint(input.position.y) % pixelSize;
    
    x = floor(pixelSize / 2.0f) - x;
    y = floor(pixelSize / 2.0f) - y;
    
    x += input.position.x;
    y += input.position.y;
    
    return Pixels.Sample(Sampler, float2(x, y) / textureSize);
}