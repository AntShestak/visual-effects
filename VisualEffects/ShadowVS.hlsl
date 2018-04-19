//this shader calculates vertice transformations 

//matrix buffer hold all required matrices for transformations
cbuffer MatrixBuffer
{
	matrix world; //objects world matrix
	matrix view; //camera view 
	matrix projection;	//camera projection
	matrix lightView; //light surce view matrix
	matrix lightProjection;	//light source projection matrix
};
//light buffer holds lights position
cbuffer LightBuffer
{
	float3 lightPosition; //light's position in the world
	float padding;
};

struct VS_Input //input structure
{
	float3 position : POSITION; //base position
	float2 tex :TEXCOORD0; //UV
	float3 normal : NORMAL; 
	float3 instPosition : TEXCOORD1; //instance position
};

struct VS_Output //output structure
{ 
	float4 position : SV_POSITION; //transformed position
	float2 tex : TEXCOORD0; //UV
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1; //position from the light's point of view
	float3 lightPosition : TEXCOORD2; //position of the light source
};

VS_Output main(VS_Input input)
{
	VS_Output output; //output strucutre
	//calculate position by adding instance position to the base
	input.position.x += input.instPosition.x;
	input.position.y += input.instPosition.y;
	input.position.z += input.instPosition.z;
	float4 position = float4(input.position, 1.0f);
	//transform position with camera matrices
	output.position = mul(position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);
	//transform position with light source matrices
	output.lightViewPosition = mul(position, world);
	output.lightViewPosition = mul(output.lightViewPosition, lightView);
	output.lightViewPosition = mul(output.lightViewPosition, lightProjection);
	//transform normal by world matrix
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	//send normalized light position to pixel shader
	output.lightPosition = normalize(lightPosition);
	output.tex = input.tex;

	return output;
}