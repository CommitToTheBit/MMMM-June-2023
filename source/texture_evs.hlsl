cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent: TANGENT;
	float3 binormal : BINORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float3 texPosition : TEXCOORD0;
};

OutputType main(InputType input)
{
	OutputType output;

	// STEP 1: Change the position vector to be 4 units for proper matrix calculations
	input.position.w = 1.0f;

	// STEP 2: Calculate the vertex's position
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// STEP 3: Set centre of model
	float3 centre = float3(0.5f, 0.5f, 0.5f); // FIXME: These should be normalised over [0.0f, 1.0f], though this is already true of marching cubes... // FIXME: Pass this in through a buffer!
	//centre = mul(centre, worldMatrix); // FIXME: Why does float3(0.33f, 0.33f, 0.33f) work for this?

	output.texPosition = input.position-centre;

	return output;
}