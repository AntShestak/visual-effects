//this simple pixel shader sets a color to a color given, transperency is taken care of with Blend State

Texture2D txDiffuse : register(t0); //texture to sample 
SamplerState samLinear : register (s0); //sampler state

struct PS_Input //input strucure 
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0; //UV
	float3 color : TEXCOORD1;
};


float4 main(PS_Input input) : SV_TARGET
{
	float4 col = float4 (input.color, 1.0f); //tranform color to float 4

	float4 ret = saturate(txDiffuse.Sample(samLinear, input.tex) * col); //sample texture & multiply by color

	return ret;
}