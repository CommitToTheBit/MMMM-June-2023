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
	float3 texPosition : TEXCOORD0;
	float3 position3D : TEXCOORD3;
	float3 normal : NORMAL;
	float3 tangent: TANGENT;
	float3 binormal : BINORMAL;
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

	// STEP 4: Calculate vertex's 3D position, normal, tangent and binormal
	output.position3D = (float3)mul(input.position, worldMatrix);

	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	output.normal *= (culling) ? 1.0f : -1.0f;

	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);
	output.tangent *= (culling) ? 1.0f : -1.0f;

	output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);
	output.binormal *= (culling) ? 1.0f : -1.0f;

	return output;
}