#pragma once

#include "DirectXWrapper.h"

//class that rendes particle system objects
class ParticleShader
{
public:
	ParticleShader(DirectXWrapper* wrapper);
	~ParticleShader();

	void Initialise(); //function to initialise all the resource
	//render function that takes objects to be rendered world matrix , camera's view & projection, objects texture
	void Render(XMMATRIX &worldMatrix, XMMATRIX &camProj, XMMATRIX &camView, ID3D11ShaderResourceView* texture);

private:

	DirectXWrapper*			m_dx = nullptr; //eference to directX

	ID3D11Buffer*			m_matrixCB = nullptr; //pointer to constant buffer
	ID3D11VertexShader*		m_particleVS = nullptr; //pointer to vertex shader
	ID3D11InputLayout*      m_particleShaderLayout = nullptr; //pointer to shader layout
	ID3D11PixelShader*		m_particlePS = nullptr; //pointer to pixel shader
	ID3D11SamplerState*		m_sampleState = nullptr; //pointer to sampler state

};