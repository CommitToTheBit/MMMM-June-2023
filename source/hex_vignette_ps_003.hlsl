#define PI 3.1415926538

Texture2D textures[1];
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

float ngon_distance(float2 st, int n)
{
    const float BANDS = 6.0f;

    float theta = atan2(st.y, st.x);

    float rotation;
    if (n%2 == 1)
        rotation = PI/2.0f;
    else if (n%4 == 2)
        rotation = PI/n;
    else
        rotation = 0.0f;

    float qtheta = theta-rotation-round((theta-rotation)/(2.0f*PI/n))*(2.0f*PI/n);

    return floor(BANDS*length(st)*cos(qtheta))/BANDS;
}

float distance(float2 st)
{
    const float TILES = 6.0f;

    st = 2.0f*st-float2(1.0f, 1.0f);
    st.x *= aspectRatio;
    st *= TILES;

    float2 ist = floor(st+float2(0.5f, 0.5f));

    float l = 2.0f*ngon_distance(st-ist, 4)/TILES;

    ist /= TILES;
    ist.x /= aspectRatio;

    st /= TILES;
    st.x /= aspectRatio;

    return length(ist)+l;
}

float4 main(InputType input) : SV_TARGET
{
    // STEP 1: Use distance metric to calculate distance from centre of screen...
    float radius = distance(input.tex); // NB: Euclidean distance metric is use here...

    // STEP 2: Calculate radial alpha of the vignette...
    float radialBound = pow(2.0f, 0.5f)-stress;
    float radialAlpha = clamp(radius-radialBound, 0.0, 1.0);

    // STEP 3: Use audio to add any bloom...
    float bpm = 120.0f; // 80.0f+140.0f*stress; a poor approximation, in that we have 'period clipping' when this isn't carefully passed in...
    float volume = abs(cos((0.5f*bpm/60.0f)*PI*time+0.25f*radius)); // NB: Substitute for actual audio being passed in...
    volume = pow(radialAlpha,1.0f)*pow(volume, 4.0f+16.0f*(1.0f-radialAlpha));

    float radialBloom = 0.05f;
    radialAlpha += radialBloom*volume;;

    // STEP 5: 
    float a = radialAlpha;
    a *= alpha;

    return max(1.0f-a, 0.0f)*textures[0].Sample(SampleType, input.tex)+a*float4(1.0f,0.0f,0.0f,1.0f);
}