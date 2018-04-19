//simple shaders that samples the texture for color

Texture2D txDiffuse; //texture to sample
SamplerState samLinear; //sampler state

struct PS_INPUT //input structure
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0; //UV
};

float4 main(PS_INPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex); //returns sampled color
}