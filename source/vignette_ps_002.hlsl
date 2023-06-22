Texture2D textures[2];
SamplerState SampleType;

cbuffer TimeBuffer : register(b1)
{
    float time;
};

cbuffer AlphaBuffer : register(b2)
{
    float intensity;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float radius = length(2.0f*input.tex-float2(1.0f,1.0f));
    float radialBound = pow(2.0f, 0.5f)-intensity;
    float radialAlpha = clamp(radius-radialBound, 0.0, 1.0);

    //float alpha = 0.5f*(0.75f*radialAlpha+0.25f*textures[1].Sample(SampleType, input.tex));
    float alpha = 0.4f*0.75f*radialAlpha+0.4f*radialAlpha*(1.0f-0.75f)*(1.0+1.0f*(0.5f+0.5f*cos(2.0f*time)*cos(2.0f*time)*cos(2.0f*time)*cos(2.0f*time)))*textures[1].Sample(SampleType, input.tex);

    return (1.0f-alpha)*textures[0].Sample(SampleType, input.tex)+alpha*float4(1.0f,0.0f,0.0f,1.0f);
}