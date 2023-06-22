#define PI 3.1415926538

Texture2D textures[2];
SamplerState SampleType;

cbuffer TimeBuffer : register(b1)
{
    float time;
};

cbuffer AlphaBuffer : register(b2)
{
    float alpha;
};

cbuffer AspectRatioBuffer : register(b4)
{
    float aspectRatio;
};

cbuffer StressBuffer : register(b5)
{
    float stress;
};


struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // STEP 1: Use a distance metric to calculate distance from centre of screen...
    float2 st = 2.0f*(input.tex-float2(0.5f,0.4f)); // NB: Shifting 'focal point' upward...
    float radius = pow(pow(length(st.x), 2.5f)+pow(length(st.y), 2.5f), 1.0f/2.5f);

    // STEP 2: Calculate radial alpha of the vignette...
    float radialBound = pow(2.0f, 0.5f)-stress;
    float radialAlpha = clamp(radius-radialBound, 0.0, 1.0);

    // STEP 3: Read arterial component from map...
    float arterialAlpha = textures[1].Sample(SampleType, input.tex);

    // STEP 4: Use audio to add any bloom...
    float bpm = 140.0f; // 80.0f+140.0f*stress; a poor approximation, in that we have 'period clipping' when this isn't carefully passed in...
    float volume = abs(cos((0.5f*bpm/60.0f)*PI*time+0.15f*radius)); // NB: Substitute for actual audio being passed in...
    volume = pow(radialAlpha,1.0f)*pow(volume, 4.0f+16.0f*(1.0f-radialAlpha));

    float radialBloom = 0.05f;
    radialAlpha += radialBloom*volume;

    float arterialBloom = radialBloom+0.5f*stress*stress;
    arterialAlpha *= 1.0f+arterialBloom*volume;

    // STEP 5: 
    float t = 1.0f/7.0f;
    float a = (1.0f-t)*max((radialAlpha-1.0f/7.0f)/(1.0f-1.0f/7.0f), 0.0f)+t*arterialAlpha;
    a *= alpha;

    //return float4(1.0f-0.4f*a, 1.0f-0.5f*a, 1.0f-0.5f*a, 1.0f); // NB: Monochrome, for presentation purposes!
    return max(1.0f-a, 0.0f)*textures[0].Sample(SampleType, input.tex)+a*float4(1.0f,0.0f,0.0f,1.0f);
}