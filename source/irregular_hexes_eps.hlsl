#define PI 3.1415926538f

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

float3 random3(float2 xy)
{
	return frac(34227.56*sin(float3(dot(xy, float2(256.3, 444.7)), dot(xy, float2(199.5, 270.4)), dot(xy, float2(390.5, 275.2)))));
}

float2 tile_st(float2 st)
{
    const int TILES_N = 3;
    const int2 TILES = int2(2*TILES_N, 2*round((2.0*TILES_N)/sqrt(3)));

    const float PERIOD = 1.0f;
    const float VARIANCE = 0.5f; // NB: Avoids division by zero!

    st.x *= TILES.x;
    st.y *= TILES.y;

    int2 ist = floor(st);
    float2 fst = frac(st);

    st *= int2(2, 2);

    // STEP 0: Find ist as triangle tile coordinates
    int idirection = (ist.x%2+2)%2;
    float iboundary = (idirection == 0) ? fst.x : 1.0-fst.x;
    if (fst.y < 0.5-0.5*iboundary)
        ist = 2*ist+int2(idirection, 0);
    else if (fst.y < 0.5+0.5*iboundary)
        ist = 2*ist+int2(1-idirection, 1);
    else
        ist = 2*ist+int2(idirection, 2);

    // STEP 1: Finding ist, the index of the distorted tile st lies in...
    idirection = (ist.x%2+2)%2;
    float2 ivertices[4];
    int2 iindices[4] = { int2(-1, -idirection), int2(1, idirection-1), int2(1-2*idirection, 1), int2(0, 0) };
    for (int i = 0; i < 4; i++)
    {
        int direction = ((idirection+iindices[i].x)%2+2)%2;
        float centre = (direction == 0) ? 1.0-0.5/tan(PI/3) : 0.5/tan(PI/3);

        float3 triangularRandomness = random3((ist+iindices[i]+TILES)%TILES);
        triangularRandomness = VARIANCE*sin(PERIOD*(1.0+length(triangularRandomness))*time+6.2831*triangularRandomness);
        triangularRandomness *= 0.5*tan(PI/6); // Keeps point within the triangle's inner circumference, for VARIANCE < 1.0

        float theta[3] = { -2+direction*PI/3, direction*PI/3, 2+direction*PI/3 };
        float2 randomness = float2(dot(triangularRandomness, float3(cos(theta[0]), cos(theta[1]), cos(theta[2]))), dot(triangularRandomness, float3(sin(theta[0]), sin(theta[1]), sin(theta[2]))));

        ivertices[i] = ist+iindices[i]+randomness+float2(centre, 0.0);
    }

    int iq = (ivertices[0].y < ivertices[3].y || idirection == 1) ? 2 : 0;
    int2 iqindices[3] = { int2(1-2*(1-idirection), -1), int2(1, idirection), int2(-1, 1-idirection) };
    float itheta = atan2(st.y-ivertices[3].y, st.x-ivertices[3].x);
    for (int i = iq; i < iq+4; i++)
    {
        if (itheta < atan2(ivertices[(i+1)%3].y-ivertices[3].y, ivertices[(i+1)%3].x-ivertices[3].x))
        {
            iq = i%3;
            break;
        }
    }
    ist += iqindices[iq]-int2(idirection, 0);

    // STEP 2: Finding fst, relative to the boundary of our (distorted) tile ist...
    float2 fvertices[6];
    int2 findices[6] = { int2(-1, 0), int2(0, -1), int2(1, -1), int2(2, 0), int2(1, 1), int2(0, 1) };
    for (int i = 0; i < 6; i++)
    {
        int direction = i%2;
        float centre = (direction == 0) ? 1.0-0.5/tan(PI/3) : 0.5/tan(PI/3);

        float3 triangularRandomness = random3((ist+findices[i]+TILES)%TILES);
        triangularRandomness = VARIANCE*sin(PERIOD*(1.0+length(triangularRandomness))*time+6.2831*triangularRandomness);
        triangularRandomness *= 0.5*tan(PI/6); // Keeps point within the triangle's inner circumference, for VARIANCE < 1.0

        float theta[3] = { -2+direction*PI/3, direction*PI/3, 2+direction*PI/3 };
        float2 randomness = float2(dot(triangularRandomness, float3(cos(theta[0]), cos(theta[1]), cos(theta[2]))), dot(triangularRandomness, float3(sin(theta[0]), sin(theta[1]), sin(theta[2]))));

        fvertices[i] = ist+findices[i]+randomness+float2(centre, 0.0);
    }
    float2 fvertexMean = (fvertices[0]+fvertices[1]+fvertices[2]+fvertices[3]+fvertices[4]+fvertices[5])/6;

    ist = floor((float2)ist/2);
    fst = float2(0.5, 0.5);
    if (length(st-fvertexMean) == 0)
        return ist+fst;

    int fq = (fvertices[0].y < fvertexMean.y) ? 5 : 0; // NB: Note assumption of low variance...
    float ftheta = atan2(st.y-fvertexMean.y, st.x-fvertexMean.x);
    for (int i = fq; i < fq+6; i++)
    {
        if (ftheta < atan2(fvertices[(i+1)%6].y-fvertexMean.y, fvertices[(i+1)%6].x-fvertexMean.x))
        {
            fq = i%6;
            break;
        }
    }

    // Finding where a line from vertexMean to st intersects with an integer edge...
    float a[2] = { (st.y-fvertexMean.y)/(st.x-fvertexMean.x), (fvertices[(fq+1)%6].y-fvertices[fq].y)/(fvertices[(fq+1)%6].x-fvertices[fq].x) };
    float b[2] = { fvertexMean.y-a[0]*fvertexMean.x, fvertices[fq].y-a[1]*fvertices[fq].x };
    float xIntersect = (b[1]-b[0])/(a[0]-a[1]);
    float yIntersect = a[0]*xIntersect+b[0];
    float2 intersect = float2(xIntersect, yIntersect);

    float outPrime = length(st-fvertexMean)/length(intersect-fvertexMean);
    float thetaPrime = atan2(intersect.y-fvertexMean.y, intersect.x-fvertexMean.x);

    float thetaRelative = acos(dot(intersect-fvertexMean, fvertices[fq]-fvertexMean)/(length(intersect-fvertexMean)*length(fvertices[fq]-fvertexMean)));
    float thetaRange = acos(dot(fvertices[(fq+1)%6]-fvertexMean, fvertices[fq]-fvertexMean)/(length(fvertices[(fq+1)%6]-fvertexMean)*length(fvertices[fq]-fvertexMean)));

    float2 f = float2(1.0, clamp(tan((thetaRelative/thetaRange-0.5)*PI/3), tan(-PI/6), tan(PI/6)));
    fst += 0.5*cos(PI/6)*outPrime*float2(f.x*cos(5*PI/6-fq*(PI/3))+f.y*sin(5*PI/6-fq*(PI/3)), -f.x*sin(5*PI/6-fq*(PI/3))+f.y*cos(5*PI/6-fq*(PI/3)));

    return ist+fst;
}

float4 main(InputType input) : SV_TARGET
{
	float3 texEnv = tex(input.texPosition);

	float2 tiled_st = tile_st(texEnv.xy);
	int2 ist = floor(tiled_st);
	float2 fst = frac(tiled_st);

	return float4(fst.x, fst.y, 0.0f, 1.0f);
}