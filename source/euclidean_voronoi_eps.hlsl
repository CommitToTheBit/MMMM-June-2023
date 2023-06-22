#define PI 3.1415926538f

#define VORONOI_TILES 4
#define VORONOI_PERIOD 15.0f
#define VORONOI_VARIANCE 0.4f

cbuffer TimeBuffer : register(b1)
{
	float time;
};

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

float2 random2(float2 xy)
{
	return frac(34227.56*sin(float2(dot(xy, float2(256.3f, 444.7f)), dot(xy, float2(199.5f, 270.4f)))));
}

float metric(float2 a, float2 b)
{
	return length(a-b);
}

float2 tile_st(float2 st)
{
	st *= VORONOI_TILES;
	int2 ist = floor(st);

	int2 closest_ist = ist;
	float closest_distance = 1.0f; // NB: Assumes VARIANCE < 0.5...
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			float2 randomness = random2((ist+int2(i, j)+int2(VORONOI_TILES, VORONOI_TILES))%VORONOI_TILES); // Modulus 'loops' our texture
			randomness = float2(0.5f, 0.5f)+VORONOI_VARIANCE*sin(2.0f*PI*(((1.0f+length(randomness))/2.0f)*((VORONOI_PERIOD > 0.0f) ? time/VORONOI_PERIOD : 0.0f)+randomness.x+randomness.y));
			float2 ivertex = ist+int2(i, j)+randomness;

			float distance = metric(st, ivertex);

			// DEBUG: Highlighting relevant points in black
			if (length(st-ivertex) < 0.0)
				return int2(-1, -1);

			if (distance < closest_distance)
			{
				closest_ist = ist+int2(i, j);
				closest_distance = distance;
			}
		}
	}
	ist = closest_ist;

	return ist;
}

float4 main(InputType input) : SV_TARGET
{
	float3 texEnv = tex(input.texPosition);

	float2 st = tile_st(texEnv.xy);
	int2 ist = floor(st);
	float2 fst = frac(st);

	return float4((ist.x+1.0f)/(VORONOI_TILES+1), (ist.y+1.0f)/(VORONOI_TILES+1), 0.2f*texEnv.z, 1.0f);
}