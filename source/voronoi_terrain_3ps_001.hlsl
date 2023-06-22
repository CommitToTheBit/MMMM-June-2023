#define PI 3.1415926538f

#define VORONOI_TILES 2
#define VORONOI_PERIOD 1.0f
#define VORONOI_VARIANCE 0.4f

cbuffer TimeBuffer : register(b1)
{
    float time;
};

cbuffer AlphaBuffer : register(b2)
{
    float alpha;
};

cbuffer LightBuffer : register(b3)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float sourceStrength;
};

struct InputType
{
    float4 position : SV_POSITION;
    float3 tex3D : TEXCOORD0;
    float3 position3D : TEXCOORD3;
    float3 normal : NORMAL;
    float3 tangent: TANGENT;
    float3 binormal : BINORMAL;
};

float3 random3(float3 xy)
{
    return frac(34227.56*sin(float3(dot(xy, float3(256.3f, 444.7f, 75.5f)), dot(xy, float3(199.5f, 270.4f, 631.3f)), dot(xy, float3(390.5f, 275.2f, 370.0f)))));
}

float metric(float3 a, float3 b)
{
    return length(a-b);
}

float3 tile_st(float3 st)
{
    st *= VORONOI_TILES;
    int3 ist = floor(st);

    int3 closest_ist = ist;
    float closest_distance = 1.0f; // NB: Assumes VARIANCE < 0.5...
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                float3 randomness = random3((ist+int3(i, j, k)+int3(VORONOI_TILES, VORONOI_TILES, VORONOI_TILES))%VORONOI_TILES); // Modulus 'loops' our texture
                randomness = float3(0.5f, 0.5f, 0.5f)+VORONOI_VARIANCE*sin(2.0f*PI*(VORONOI_PERIOD*(1.0f+length(randomness))*time+randomness));
                float3 ivertex = ist+int3(i, j, k)+randomness;

                float distance = metric(st, ivertex);

                // DEBUG: Highlighting relevant points in black
                if (length(st-ivertex) < 0.0)
                    return int3(-1, -1, -1);

                if (distance < closest_distance)
                {
                    closest_ist = ist+int3(i, j, k);
                    closest_distance = distance;
                }
            }
        }
    }
    ist = closest_ist;

    return ist;
}

float4 voronoi(float3 st)
{
    st = tile_st(st);
    int3 ist = floor(st);
    float3 fst = frac(st);

    return float4(0.50f+0.03f*(0.5f+0.5f*sin(2.0f*PI*(ist.x+1.0f)/(VORONOI_TILES+1))), 0.51f-0.01f*(0.5f+0.5f*sin(2.0f*PI*(ist.y+1.0f)/(VORONOI_TILES+1))), 0.53f-0.04f*(0.5f+0.5f*sin(2.0f*PI*(ist.z+1.0f)/(VORONOI_TILES+1))), 1.0f);
}

float4 main(InputType input) : SV_TARGET
{
    // STEP 1: Sample from the voronoi texture to calculate the pixel's base colour
    float4 textureColor = voronoi(input.tex3D);

    // STEP 2: Calculate lighting hitting pixel
    float3 lightDirection = normalize(input.position3D - lightPosition);
    float3 lightDistance = max(length(input.position3D - lightPosition), pow(sourceStrength, 0.5));
    //float lightIntensity = saturate(dot(input.normal, -lightDirection)); // NB: Ignores source strength...?
    float lightIntensity = sourceStrength*saturate(dot(input.normal, -lightDirection))/(lightDistance*lightDistance);
    float4 lightColor = ambientColor + diffuseColor * lightIntensity;
    lightColor = saturate(lightColor);

    // STEP 3: Applying lighting to pixel's base colour.
    float4 color = lightColor * textureColor;
    color.a = alpha;

    return color;
}
