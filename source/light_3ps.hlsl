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

float4 main(InputType input) : SV_TARGET
{
    // STEP 1: Set the pixel's base colour
    float4 textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    // STEP 2: Calculate lighting hitting pixel
    float3 lightDirection = normalize(input.position3D - lightPosition);
    float3 lightDistance = max(length(input.position3D - lightPosition), pow(sourceStrength, 0.5));
    float lightIntensity = sourceStrength*saturate(dot(input.normal, -lightDirection))/(lightDistance*lightDistance);
    float4 lightColor = ambientColor + diffuseColor * lightIntensity;
    lightColor = saturate(lightColor);

    // STEP 3: Applying lighting to pixel's base colour.
    float4 color = lightColor * textureColor;
    color.a = alpha;

    return color;
}
