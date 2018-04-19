#pragma once


#include "DirectXWrapper.h"
//class that set shaders to render GUI objects
class UIShader
{
public:
	UIShader(DirectXWrapper *wrapper);
	~UIShader();

	void Initialise(); //funtion that initialises shaders and buffer
	//render funtion takes objects being rendered world matrix & texture
	void Render(XMMATRIX &world, ID3D11ShaderResourceView* texture);

private:
	DirectXWrapper*			wrapper = nullptr; //reference to diect X
	
	ID3D11Buffer*			m_constantBuffer = nullptr;	//contant buffer pointer
	ID3D11VertexShader*		m_UIVertexShader = nullptr;	//vertex shader pointer
	ID3D11InputLayout*      m_shaderLayout = nullptr;	//shader lyout pointer
	ID3D11PixelShader*		m_UIPixelShader = nullptr;	//pixel shader pointer
	ID3D11SamplerState*		m_samplerLinear = nullptr; //sampler state pointer

};