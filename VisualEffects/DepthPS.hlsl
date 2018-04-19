//this shader writes pixel depth value to a texture

struct PS_INPUT //input structure
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//calculate pixel depth value by dividing depth pixel Z by W
	float depthValue = input.depthPosition.z / input.depthPosition.w;

	return float4(depthValue, depthValue, depthValue, 1.0f); //assign depth value to all RGB values
}