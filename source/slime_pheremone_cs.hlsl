#define PI 3.1415926538f

cbuffer SlimeBuffer : register(b0)
{
    int species;
    int cells;
    float speed;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float delta;
};

cbuffer TextureBuffer : register(b3)
{
    int width;
    int height;
};

struct Species
{
    float4 colour;

    float3 sensorMask;
    float3 sensorAttraction;

    float speed;
    float angularVelocity;

    float sensorDistance;
    float sensorAngle;
    float sensorPixelRadius;

    float pheremoneDispersion;
};
StructuredBuffer<Species> InputSpeciesBuffer : register(t0);

Texture2D<float4> InputTexture : register(t2);
Texture2D<float4> InputPheremoneMask : register(t3);
Texture2D<float4> InputSpeciesMaskA : register(t4);
Texture2D<float4> InputSpeciesMaskB : register(t5);
Texture2D<float4> InputSpeciesMaskC : register(t6);

RWTexture2D<float4> OutputTexture : register(u2);
RWTexture2D<float4> OutputPheremoneMask : register(u3);

[numthreads(16, 16, 1)]
void main(int3 ID : SV_DispatchThreadID)
{
    // STEP 1: Diffuse phremones...
    // NB: Use a Gaussian blur for this?
    float4 pheremoneMaskMean = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int offsetX = -1; offsetX <= 1; offsetX++) 
    {
        for (int offsetY = -1; offsetY <= 1; offsetY++) 
        {
            int sampleX = min(width-1, max(0, ID.x + offsetX));
            int sampleY = min(height-1, max(0, ID.y + offsetY));
            
            pheremoneMaskMean += InputPheremoneMask[int2(sampleX, sampleY)];
        }
    }
    pheremoneMaskMean /= 9.0f;

    float diffusion = saturate(10.0f * delta * speed);
    float4 pheremoneMaskDiffuse = (1 - diffusion) * InputPheremoneMask[ID.xy] + diffusion * pheremoneMaskMean;

    // STEP 2: Evaporate phremones...
    float4 pheremoneMaskEvaporate = saturate(pheremoneMaskDiffuse - 10.0f * delta * speed * float4(1.0f, 1.0f, 1.0f, 0.0f)); // NB: Scaled to speed...

    // STEP 3: Add agents...

    // DEBUG: Hacky 'eating food' effect...
    OutputTexture[ID.xy] = float4(1.0f, 1.0f, 1.0f, InputTexture[ID.xy].a);

    if (length(InputSpeciesMaskA[ID.xy].rgb) > 0.0f || length(InputSpeciesMaskB[ID.xy].rgb) > 0.0f || length(InputSpeciesMaskC[ID.xy].rgb) > 0.0f)
        OutputTexture[ID.xy] = float4(0.0f, 0.0f, 0.0f, 1.0f);

    OutputPheremoneMask[ID.xy] = pheremoneMaskEvaporate;
    if (length(InputSpeciesMaskA[ID.xy].rgb) > 0.0f || length(InputSpeciesMaskB[ID.xy].rgb) > 0.0f || length(InputSpeciesMaskC[ID.xy].rgb) > 0.0f)
        OutputPheremoneMask[ID.xy] = InputSpeciesMaskA[ID.xy] + InputSpeciesMaskB[ID.xy] + InputSpeciesMaskC[ID.xy];
}