//this shader desided if pixel is lighted or shadowed by comparing depth position

Texture2D objectTexture : register(t0); //objects texture to sample pixel color
Texture2D depthMapTexture : register(t1); //depth map texture to sample depth values

SamplerState SampleStateClamp : register (s0); //sampler state with CLAMP address mode

struct PS_Input //input structure
{
	float4 position : SV_POSITION; //positioned from camera's point of view
	float2 tex : TEXCOORD0; //UV
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1; //position from light's point of view
	float3 lightPos: TEXCOORD2; //world position of the light
};

float4 main(PS_Input input) : SV_TARGET
{
	float2 projectTexCoord; //projected texture coordinates
	float depthValue;	//sampled depth value
	float lightDepthValue; //depth value to compare
	float lightIntensity;	
	float4 textureColor;
	//set colors here
	float4 ambientColor = float4 (0.5, 0.5, 0.5, 1.0);
	float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
	float4 color = ambientColor; //set final color to ambient
	//bias sed to get rid of shadow acne
	float bias = 0.001f;
	//obtin projected texture coordinates
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	//if projected texture coordinates are in range of 0 and 1. 
	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y == projectTexCoord.y)))
	{
		//pixel can be seen by light
		
		depthValue = depthMapTexture.Sample(SampleStateClamp,projectTexCoord).r; //sample depth value

		lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w; //calculate ight depth value
		lightDepthValue = lightDepthValue - bias; //fixing acne
		//comparing obtained depth values
		if (lightDepthValue < depthValue)
		{
			float3 norm = normalize(input.normal);
			//calculate the amount of light
			lightIntensity = saturate(dot(norm, input.lightPos));
			//now diffuse light calculation
			if (lightIntensity > 0)
			{
				color += (diffuseColor * lightIntensity);
				color = saturate(color);
			}
		
		}	
	}
	
	textureColor = objectTexture.Sample(SampleStateClamp,input.tex); //sample texture

	return color * textureColor; //return color multiplied by sampled texture color
}

