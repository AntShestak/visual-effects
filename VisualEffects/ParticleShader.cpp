#include "stdafx.h"
#include "ParticleShader.h"
//include compiled shader files
#include "CompiledParticleVertexShader.h"
#include "CompiledParticlePixelShader.h"

//matrix constant buffer structure
struct MatrixBuffer
{
	XMMATRIX worldMatrix; //objects world matrix
	XMMATRIX view; //camera view matrix
	XMMATRIX proj;	//camera projection matrix
};

ParticleShader::ParticleShader(DirectXWrapper* wrapper)
{
	m_dx = wrapper; //reference to directX
	Initialise();	//call initialise
}

ParticleShader::~ParticleShader()
{
	m_dx = nullptr; //null a pointer
	//release resources & null pointers
	if (m_matrixCB) m_matrixCB->Release();
	m_matrixCB = nullptr;
	if (m_particleVS) m_particleVS->Release();
	m_particleVS = nullptr;
	if (m_particleShaderLayout) m_particleShaderLayout->Release();
	m_particleShaderLayout = nullptr;
	if (m_particlePS) m_particlePS->Release();
	m_particlePS = nullptr;
	if (m_sampleState) m_sampleState->Release();
	m_sampleState = nullptr;
}

void ParticleShader::Initialise()
{
	// creating vertex shader from compiled file
	HRESULT hr = m_dx->GetDevice()->CreateVertexShader(g_CompiledParticleVertexShader, sizeof(g_CompiledParticleVertexShader), nullptr, &m_particleVS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex shader");
		return;
	}

	// create pixel shader
	hr = m_dx->GetDevice()->CreatePixelShader(g_CompiledParticlePixelShader, sizeof(g_CompiledParticlePixelShader), nullptr, &m_particlePS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create pixel shader");
		return;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//objects position
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//uv
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//normal
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	//instance positions
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	//instance colors
	};
	UINT numElements = ARRAYSIZE(layout);
	// Create the input layout  
	hr = m_dx->GetDevice()->CreateInputLayout(layout, numElements, g_CompiledParticleVertexShader, sizeof(g_CompiledParticleVertexShader), &m_particleShaderLayout);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create input layout");
		return;
	}

	//define sampler state for WRAP address mode
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//create sampler state
	hr = m_dx->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create sampler state wrap");
		return;
	}

	//Creating constant buffers
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatrixBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_dx->GetDevice()->CreateBuffer(&bd, nullptr, &m_matrixCB);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create particle shader constant buffer");
		return;
	}

	return;
}
//render function that takes objects to be rendered world matrix , camera's view & projection, objects texture
void ParticleShader::Render(XMMATRIX &worldMatrix, XMMATRIX &camProj, XMMATRIX &camView, ID3D11ShaderResourceView* texture)
{
	XMMATRIX world = worldMatrix; //objects world matrix
	XMMATRIX view = camView;	//camera view matrix
	XMMATRIX proj = camProj;	//camera projection matrix

	MatrixBuffer cb;
	//initilise buffer
	//transpose matrices as HLSL shaders use different matrix layout
	cb.worldMatrix = XMMatrixTranspose(world);
	cb.view = XMMatrixTranspose(view);
	cb.proj = XMMatrixTranspose(proj);
	//update buffer
	m_dx->GetContext()->UpdateSubresource(m_matrixCB, 0, nullptr, &cb, 0, 0);
	//send it to shader
	m_dx->GetContext()->VSSetConstantBuffers(0, 1, &m_matrixCB);
	//passing texture to shaderr
	m_dx->GetContext()->PSSetShaderResources(0, 1, &texture);
	//setting sampler
	m_dx->GetContext()->PSSetSamplers(0, 1, &m_sampleState);

	// Set the input layout
	m_dx->GetContext()->IASetInputLayout(m_particleShaderLayout);
	// Set the vertex and pixel shaders
	m_dx->GetContext()->VSSetShader(m_particleVS, nullptr, 0);
	m_dx->GetContext()->PSSetShader(m_particlePS, nullptr, 0);

	//draw is called by object to be rendered when this function returns
	return;
}