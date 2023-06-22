#define PI 3.1415926538f

cbuffer SlimeBuffer : register(b0)
{
    int species;
    int cells;
    float speed;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float delta;
};

cbuffer TextureBuffer : register(b3)
{
    int width;
    int height;
};

struct Species
{
    float4 colour;

    float3 sensorMask;
    float3 sensorAttraction;

    float speed;
    float angularVelocity;

    float sensorDistance;
    float sensorAngle;
    float sensorPixelRadius;

    float pheremoneDispersion;
};
StructuredBuffer<Species> InputSpeciesBuffer : register(t0);

struct Cell
{
    int species;
    float2 position;
    float angle;
};
StructuredBuffer<Cell> InputCellBuffer : register(t1);
RWStructuredBuffer<Cell> OutputCellBuffer : register(u1);

Texture2D<float4> InputPheremoneMask : register(t3);
Texture2D<float4> InputFoodMask : register(t7);

RWTexture2D<float4> OutputSpeciesMaskA : register(u4);
RWTexture2D<float4> OutputSpeciesMaskB : register(u5);
RWTexture2D<float4> OutputSpeciesMaskC : register(u6);
RWTexture2D<float4> OutputFoodMask : register(u7);


/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
/* This enclosed section has been adapted from: Sebastian Lague (2021) Slime Simulation. Available at https://github.com/SebLague/Slime-Simulation (Accessed: 13 June 2022) */

uint hash(uint n)
{
    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    /* This enclosed section has been copied from: Hagit Schechter and Robert Brigson (2008) Evolving Sub-Grid Turbulence for Smoke Animation. Available at https://www.cs.ubc.ca/~rbridson/docs/schechter-sca08-turbulence.pdf (Accessed: 13 June 2023) */
    
    n ^= 2747636419u;
    n *= 2654435769u;
    n ^= n >> 16;
    n *= 2654435769u;
    n ^= n >> 16;
    n *= 2654435769u;

    return n;

    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
}

float fHash(uint n)
{
    return ((float)hash(n)) / 4294967295.0f;
}

float random(uint2 id)
{
    return fHash(id.y * (width+id.y) + id.x + hash(id.x + time * 100000));
}

float2 rotate(float2 v, float deltaTheta)
{
    float r = length(v);
    float theta = atan2(v.y, v.x);

    return float2(r*cos(theta+deltaTheta), r*sin(theta+deltaTheta));
}

float sense(Cell cell, Species cellSpecies, float direction)
{
    float sensorAngle = cell.angle + direction * cellSpecies.sensorAngle;
    float2 sensorDir = float2(cos(sensorAngle), sin(sensorAngle));

    float sensorOffsetDst = cellSpecies.sensorDistance; // NB: Essential for controlling turn size!  // *width;// 1.0f * length(agent.velocity);
    float2 sensorPos = cell.position + sensorDir * sensorOffsetDst;
    int sensorCentreX = round(width*sensorPos.x);
    int sensorCentreY = round(height*sensorPos.y);
    
    float sum = 0.0f;

    // FIXME: Inelegant 'bound' forcing ants inward... how could this be handled neatly/more generally?
    float boundary = 0.01f;
    if (sensorPos.x < boundary || sensorPos.x >= 1.0f - boundary || sensorPos.y < boundary || sensorPos.y >= 1.0f - boundary)
        sum -= 100.0f * length(sensorPos - float2(0.5f, 0.5f));
    
    // ...
    float placement = dot(cellSpecies.sensorAttraction, (length(sensorPos - float2(0.42f, 0.4f)) > 0.36f) ? float3(1.0f, -1.0f, -1.0f) : float3(-1.0f, 1.0f, 1.0f));
    if ((length(sensorPos - float2(0.42f, 0.4f)) > 0.36f && placement <= 0.0f) || placement < 0.0f)
        sum -= 0.9f * ((length(sensorPos - float2(0.42f, 0.4f)) > 0.36f) ? 1.0f : -1.0f * length(placement)) * length(sensorPos - float2(0.42f, 0.4f));

    // ...
    //float placement = dot(cellSpecies.sensorAttraction, InputFoodMask[int2(sensorCentreX, sensorCentreY)]);
    //if (false)//placement <= 0.0f)
    // Red in, green out, blue out...
    //    sum -= 100.0f * (InputFoodMask[int2(sensorCentreX, sensorCentreY)].r - InputFoodMask[int2(sensorCentreX, sensorCentreY)].g - InputFoodMask[int2(sensorCentreX, sensorCentreY)].b) * length(sensorPos - float2(0.5f, 0.5f));

    int sensorSize = cellSpecies.sensorPixelRadius;// 4;// 0.025f * width;
    for (int offsetX = -sensorSize; offsetX <= sensorSize; offsetX++) 
    {
        for (int offsetY = -sensorSize; offsetY <= sensorSize; offsetY++) 
        {
            int sampleX = min(width - 1, max(0, sensorCentreX + offsetX));
            int sampleY = min(height - 1, max(0, sensorCentreY + offsetY));
            sum += dot(cellSpecies.sensorAttraction, InputPheremoneMask[int2(sampleX, sampleY)]);
        }
    }

    return sum;
}


[numthreads(16, 1, 1)]
void main(int3 ID : SV_DispatchThreadID)
{
    if (ID.x >= cells)
        return;

    Cell cell = InputCellBuffer[ID.x];

    int2 inputTex = int2(round(width * InputCellBuffer[ID.x].position.x), round(height * InputCellBuffer[ID.x].position.y));
    if (time == 0.0f)
    {
        cell.position = float2(0.5f, 0.5f) + rotate(cell.position-float2(0.5f, 0.5f), random(inputTex + int2(ID.x, ID.x)) * (2.0f * PI));
        //cell.position = float2(0.5f, 0.5f) + random(inputTex + int2(ID.x, ID.x)) * rotate(cell.position-float2(0.5f, 0.5f), random(inputTex + int2(ID.x, ID.x)) * (2.0f * PI));

        cell.angle = PI + atan2(cell.position.y-0.5f, cell.position.x-0.5f);
    }

    //if (false)
    //if (ID.x >= (time/180.0f) * cells)
    if (time < 15.0f || ID.x >= cells * ((time - 15.0f) / 60.0f) * ((time - 15.0f) / 60.0f))
    {
        OutputCellBuffer[ID.x] = cell;
        return;
    }

    Species cellSpecies = InputSpeciesBuffer[cell.species%species];
    float cellSpeed = speed * cellSpecies.speed;

    // NB: Randomise!
    /**/

    // STEER AGENT
    // Steer based on sensory data
    //float sensorAngleRad = 30.0f * (PI / 180.0f);// (45.0f + 30.0f * sin(2.0f * PI * time / 6.0f)) * (PI / 180.0f);
    float weightForward = sense(cell, cellSpecies, 0.0f);
    float weightLeft = sense(cell, cellSpecies, 1.0f);
    float weightRight = sense(cell, cellSpecies, -1.0f);

    float randomSteerStrength = fHash(random(inputTex));// +random(int2(ID.x, ID.x)));
    float turnSpeed = cellSpecies.angularVelocity * (2.0f * PI);

    // Continue in same direction
    if (weightForward > weightLeft && weightForward > weightRight) {
        cell.angle += 0.0f;
    }
    else if (weightForward < weightLeft && weightForward < weightRight) {
        cell.angle += (2.0f * randomSteerStrength - 1.0f) * delta * turnSpeed;
    }
    // Turn right
    else if (weightRight > weightLeft) {
        cell.angle += -randomSteerStrength * delta * turnSpeed;
    }
    // Turn left
    else if (weightLeft > weightRight) {
        cell.angle +=  randomSteerStrength * delta * turnSpeed;
    }

    // MOVE AGENT...
    cell.position += delta * cellSpeed * float2(cos(cell.angle), sin(cell.angle));

    // BOUNCE AGENT...
    if (cell.position.x < 0.0f || cell.position.x >= 1.0f || cell.position.y < 0.0f || cell.position.y >= 1.0f)
    {
        cell.position.x = clamp(cell.position.x, 0.0f, 1.0f-1.0f/width);
        cell.position.y = clamp(cell.position.y, 0.0f, 1.0f-1.0f/height);

        cell.angle += PI + (2.0f * random(inputTex) - 1.0f) * (PI / 6.0f);

        // DEBUG:
        //agent.velocity = rotate(agent.velocity, 2.0f * PI * random(inputTex));
        //agent.velocity = rotate(agent.velocity, 2.0f * PI * random(random(random(inputTex)) + int2(ID.x, ID.x)));
        //agent.velocity = rotate(agent.velocity, 6.0f * (PI/ 6.0f) + 0.0f * (PI / 3.0f) * random(inputTex));
    }

    // UPDATE BUFFERS...
    OutputCellBuffer[ID.x] = cell;

    int2 outputTex = int2(round(width * OutputCellBuffer[ID.x].position.x), round(height * OutputCellBuffer[ID.x].position.y));
    if (cell.species%3 == 0)
    {
        OutputSpeciesMaskA[outputTex] = float4(cellSpecies.sensorMask, 1.0f);
    }
    else if (cell.species%3 == 1)
    {
        OutputSpeciesMaskB[outputTex] = float4(cellSpecies.sensorMask, 1.0f);
    }
    else
    {
        OutputSpeciesMaskC[outputTex] = float4(cellSpecies.sensorMask, 1.0f);
    }

    // DEBUG:
    //OutputSpeciesMaskA[outputTex] = InputSpeciesBuffer[cell.species].colour;
    OutputFoodMask[outputTex] = float4(cellSpecies.sensorMask, 1.0f);// InputFoodMask[outputTex];
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */