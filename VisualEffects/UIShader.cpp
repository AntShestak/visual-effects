#include "stdafx.h"
#include "UIShader.h"

#include "CompiledUIPixelShader.h"
#include "CompiledUIVertexShader.h"

//constant buffer
struct ConstantBuffer
{
	XMMATRIX worldMatrix; //objects world matrix
};

UIShader::UIShader(DirectXWrapper * dx) //contructor
{
	wrapper = dx; //reference to directX
	Initialise();	//call initialise
}

UIShader::~UIShader() //destructor
{
	wrapper = nullptr; //null a pointer
	//release resource & null their pointers
	if (m_constantBuffer) m_constantBuffer->Release();
	m_constantBuffer = nullptr;
	if (m_UIVertexShader) m_UIVertexShader->Release();
	m_UIVertexShader = nullptr;
	if (m_shaderLayout) m_shaderLayout->Release();
	m_shaderLayout = nullptr;
	if (m_UIPixelShader) m_UIPixelShader->Release();
	m_UIPixelShader = nullptr;
	if (m_samplerLinear) m_samplerLinear->Release();
	m_samplerLinear = nullptr;
}

void UIShader::Initialise()
{
	// create vertex shader
	HRESULT hr = wrapper->GetDevice()->CreateVertexShader(g_CompiledUIVertexShader, sizeof(g_CompiledUIVertexShader), nullptr, &m_UIVertexShader);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex shader");
	}

	// create pixel shader
	hr = wrapper->GetDevice()->CreatePixelShader(g_CompiledUIPixelShader, sizeof(g_CompiledUIPixelShader), nullptr, &m_UIPixelShader);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create pixel shader");
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//position of the object
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, //uv
	};
	UINT numElements = ARRAYSIZE(layout);
	// Create the input layout
	wrapper->GetDevice()->CreateInputLayout(layout, numElements, g_CompiledUIVertexShader, sizeof(g_CompiledUIVertexShader), &m_shaderLayout);

	// Create the constant buffer used in the vertex shader
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = wrapper->GetDevice()->CreateBuffer(&bd, nullptr, &m_constantBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer");
		return;
	}

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = wrapper->GetDevice()->CreateSamplerState(&sampDesc, &m_samplerLinear);

	return;
}

void UIShader::Render(XMMATRIX &world, ID3D11ShaderResourceView* resource) //render function takes objects to render worldMatrix & texture
{
	//for GUI objects i don't need projection and view matrices as they are not transformed in that way but are simply rendered to screen in 2D style on top of other world objects
	ConstantBuffer cb;
	cb.worldMatrix = XMMatrixTranspose(world); // transpose because HLSL shaders use different matrix layout
	wrapper->GetContext()->UpdateSubresource(m_constantBuffer, 0, nullptr, &cb, 0, 0); //update buffer
	wrapper->GetContext()->VSSetConstantBuffers(0, 1, &m_constantBuffer); //send it to shader

	wrapper->GetContext()->PSSetShaderResources(0, 1, &resource); //send texture to shader
	wrapper->GetContext()->PSSetSamplers(0, 1, &m_samplerLinear);	//set sampler
	// Set the input layout
	wrapper->GetContext()->IASetInputLayout(m_shaderLayout);

	// Set the vertex and pixel shaders
	wrapper->GetContext()->VSSetShader(m_UIVertexShader, nullptr, 0);
	wrapper->GetContext()->PSSetShader(m_UIPixelShader, nullptr, 0);

	//draw called is issued by object to be rendered after this function returns
	return;
}
