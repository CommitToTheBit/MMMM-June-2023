struct InputType
{
    float4 position : SV_POSITION;
    float3 tex3D : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    return float4(input.tex3D.x, input.tex3D.y, input.tex3D.z, 1.0f);
}