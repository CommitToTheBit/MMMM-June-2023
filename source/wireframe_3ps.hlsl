cbuffer AlphaBuffer : register(b2)
{
    float alpha;
};

struct InputType
{
    float4 position : SV_POSITION;
    float3 tex3D : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float3 st = 2.0f*input.tex3D-float3(1.0f,1.0f,1.0f);

    float3 frame = float3(min(length(st.y), length(st.z)), min(length(st.z), length(st.x)), min(length(st.x), length(st.y)));
    if (max(length(frame.x), max(length(frame.y), length(frame.z))) > 0.9f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}