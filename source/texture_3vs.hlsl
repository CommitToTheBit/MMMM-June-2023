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
	float3 tex3D : TEXCOORD0;
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

	// STEP 3: Set voxel texture coordinates
	output.tex3D = input.position; // FIXME: These should be normalised over [0.0f, 1.0f], though this is already true of marching cubes...
	output.tex3D.y = 1.0f-output.tex3D.y;

	return output;
}