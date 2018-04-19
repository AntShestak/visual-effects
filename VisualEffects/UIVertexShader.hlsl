//this shader transforms vertex only by objects world matrix only

//constant buffer
cbuffer ConstantBuffer
{
	matrix Projection; //object's world matrix
}

struct VS_INPUT //input structure
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD; //UV
};

struct PS_INPUT //output structure
{
	float4 Pos : SV_POSITION; //transformed position
	float2 Tex : TEXCOORD0; //UV
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output; //output structure
	input.Pos.w = 1.0f;
	//transform position
	output.Pos = mul(input.Pos, Projection);
	//send UV to pixel shader
	output.Tex = input.Tex;
	
	return output;
}