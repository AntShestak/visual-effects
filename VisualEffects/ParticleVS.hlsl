//this shader transforms vertex and passes it to pixel shader as well as normal & color

//matrix constant buffer holds all matrices reuired for transformations
cbuffer MatrixBuffer
{
	matrix world; //objects world matrix
	matrix view; //camera view and projection
	matrix proj;
};
 
struct VS_Input //input structure
{
	float3 position : POSITION;  //base position
	float2 tex :TEXCOORD0; //UV
	float3 normal : NORMAL;
	float3 instPosition : TEXCOORD1; //instance position
	float3 instColor : TEXCOORD2; //instance color
};

struct VS_Output //output scturecture
{
	float4 position : SV_POSITION;//transformed position
	float2 tex : TEXCOORD0;	//UV
	float3 color : TEXCOORD1;
};

VS_Output main(VS_Input input)
{
	VS_Output output; //output structure
	//calculate position by adding instance position to the base class
	input.position.x += input.instPosition.x;
	input.position.y += input.instPosition.y;
	input.position.z += input.instPosition.z;
	float4 position = float4(input.position, 1.0f);
	//transform position using objects world matrix & camera matrices
	output.position = mul(position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);
	//assign UV to output structure
	output.tex = input.tex;
	//send color to pixel shader
	output.color = input.instColor;
	
	return output;
}