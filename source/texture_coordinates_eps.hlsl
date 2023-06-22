#define PI 3.1415926538f

struct InputType
{
    float4 position : SV_POSITION;
	float3 texPosition : TEXCOORD0;
};

float3 tex(float3 relative)
{ 
	float extremity = max(length(relative.x), max(length(relative.y), length(relative.z)));
	float extremities[6] = { -relative.x, relative.z, relative.x, -relative.z, relative.y, -relative.y };
	float2 sts[6] = {
		float2(relative.z, -relative.y),
		float2(relative.x, -relative.y),
		float2(-relative.z, -relative.y),
		float2(-relative.x, -relative.y),
		float2(relative.x,  relative.z),
		float2(relative.x, -relative.z)
	};

	for (int i = 0; i < 6; i++)
	{
		if (extremity == extremities[i])
		{
			float2 st = sts[i]/extremity;
			if (length(st) > 0.0f)
			{
				float2 stEdge = st/max(length(st.x), length(st.y));
				float stProportion = length(st)/length(stEdge);
				st = (sin(stProportion*PI/3.0f)/sin(PI/3.0f))*stEdge;

				st = 0.5*(st+float2(1.0f, 1.0f));
				return float3(st.x, st.y, i);
			}
			else
			{
				return float3(0.5f, 0.5f, i);
			}
		}
	}

	return float3(0.0f, 0.0f, 0.0f);
}

float4 main(InputType input) : SV_TARGET
{
	float3 texEnv = tex(input.texPosition);

    return float4(texEnv.x, texEnv.y, 0.2f*texEnv.z, 1.0f);
}