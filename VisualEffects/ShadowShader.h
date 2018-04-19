#pragma once

#include "Light.h" 
#include "DirectXWrapper.h"
//class that sets shaders for rendering with shadows
class ShadowShader
{
public:
	ShadowShader(DirectXWrapper *wrapper); //constructor
	~ShadowShader();	//destructor
	
	void Initialise(); //initialises class
	//render function sets shaders
	//worldMatrix - objects matrix , camera's view & projection, light's view & projection matrices, objects texture resource, light reference
	void Render(XMMATRIX &worldMatrix, XMMATRIX &view, XMMATRIX &projection, XMMATRIX &lightView, XMMATRIX &lightProjection, ID3D11ShaderResourceView* texture, Light* light);

private:
	DirectXWrapper*			m_dx = nullptr; //reference to directX
	
	ID3D11VertexShader*		m_shadowVS = nullptr;			//vertex shader pointer
	ID3D11InputLayout*      m_shadowShaderLayout = nullptr;	//shader layout pointer
	ID3D11PixelShader*		m_shadowPS = nullptr;			//pixel shader pointer
		
	ID3D11SamplerState* m_sampleStateClamp = nullptr;	//sample state for texture sampling
	//constant buffers, better described in .cpp
	ID3D11Buffer* m_matrixBuffer = nullptr;	
	ID3D11Buffer* m_lightBuffer = nullptr;
};