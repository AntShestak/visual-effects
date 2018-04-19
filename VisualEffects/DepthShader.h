#pragma once

#include "DirectXWrapper.h"
//class that sets shaders to render to texture
class DepthShader
{
public:
	DepthShader(DirectXWrapper *wrapper); //constructor
	~DepthShader();		//destructor

	void Initialise(); //function for initialisation
	void Render(XMMATRIX &world,XMMATRIX &view, XMMATRIX &projection); //render takes objects world matrix, and light's view & projection

private:
	DirectXWrapper*			m_dx = nullptr; //reference to direct x

	ID3D11VertexShader*		m_depthVS = nullptr;	//vertex shader
	ID3D11InputLayout*      m_depthShaderLayout = nullptr; //shader input layout
	ID3D11PixelShader*		m_depthPS = nullptr;	//pixel shader
	ID3D11Buffer*			m_matrixBuffer = nullptr;	//constant buffer
};