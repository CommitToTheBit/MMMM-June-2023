cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	bool culling;
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

	return output;
}