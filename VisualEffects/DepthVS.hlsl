//this vertex shader tranforms vertices and prepares them for pixel shader

//buffer with all required matrices
cbuffer MatrixBuffer
{
	matrix world; //objects world matrix
	matrix view; //camera matrices
	matrix projection;
}

struct VS_INPUT
{
	float4 position : POSITION; //base position
	float3 instPosition : TEXCOORD;	//instance position
};

struct PS_INPUT //output structure
{
	float4 position : SV_POSITION; //position
	float4 depthPosition : TEXTURE0;	//depth position for depth value calculations
};

PS_INPUT main(VS_INPUT input)
{
	//calculate position by adding instance position to the base
	PS_INPUT output; //output strucute
	input.position.x += input.instPosition.x;
	input.position.y += input.instPosition.y;
	input.position.z += input.instPosition.z;
	input.position.w = 1.0f;
	//multily vertex by all matrices
	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);
	//write position into depthPosition
	output.depthPosition = output.position;

	return output;
}