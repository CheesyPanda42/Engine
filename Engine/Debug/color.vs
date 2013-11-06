// Color.vs
// Shader file

// Globals
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
}

// Type defs
struct VertexInputType
{
	float4 position : POSITION;
	float4 color    : COLOR;
}

// Vertex Shader
PixelInputType ColorVertexShader(VertexInputType input)
{
	PixelInputType output;
	
	// Change position vector
	input.position.w = 1.0f;
	
	// Calculate position of vertex against world, view, and proj matricies
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	
	// Store input color for pixel shader
	output.color = input.color;
	
	return output;
}



