#include "stdafx.h"
#include "ShadowShader.h"
//include compiled shader files
#include "CompiledShadowVS.h"
#include "CompiledShadowPS.h"

//vertex shader constant buffer 1
struct MatrixBufferType
{
	XMMATRIX world; //world matrix of current object being redered
	XMMATRIX view;		//camera view matrix
	XMMATRIX projection;	//camera projection maxtrix
	XMMATRIX lightView; //light view maxtrix
	XMMATRIX lightProjection;	//light projection matrix
};

//vertex shader constant buffer 2
struct LightBufferType
{
	XMFLOAT3 lightPosition; //lights position
	float padding;	//float , because lightPosition is float3, but I need float4 alignment
};

//consructor
ShadowShader::ShadowShader(DirectXWrapper * dx)
{
	m_dx = dx;	//set directX reference
	Initialise(); //call initialise
}

ShadowShader::~ShadowShader()
{
	//release all the resources
	if (m_shadowVS) m_shadowVS->Release();
	if (m_shadowShaderLayout) m_shadowShaderLayout->Release();
	if (m_shadowPS) m_shadowPS->Release();
	if (m_sampleStateClamp)m_sampleStateClamp->Release();
	if (m_matrixBuffer)m_matrixBuffer->Release();
	if (m_lightBuffer)m_lightBuffer->Release();
}

void ShadowShader::Initialise()
{
	//creating shadow shaders
	// creating vertex shader, it's precompiled
	HRESULT hr = m_dx->GetDevice()->CreateVertexShader(g_CompiledShadowVS, sizeof(g_CompiledShadowVS), nullptr, &m_shadowVS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex shader");
		return;
	}

	// creating pixel shader it's precompiled
	hr = m_dx->GetDevice()->CreatePixelShader(g_CompiledShadowPS, sizeof(g_CompiledShadowPS), nullptr, &m_shadowPS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create pixel shader");
		return;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, //position of object
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//uv
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//normal
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, //instance positions
	};
	UINT numElements = ARRAYSIZE(layout);
	// Create the input layout  
	hr = m_dx->GetDevice()->CreateInputLayout(layout, numElements, g_CompiledShadowVS, sizeof(g_CompiledShadowVS), &m_shadowShaderLayout);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create input layout");
		return;
	}

	//define sampler state usin CLAMP address mode
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//crete sampler state
	hr = m_dx->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create sampler state clamp");
		return;
	}

	//Creating constant buffers
	//vertex shader constant buffer 1
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatrixBufferType);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_dx->GetDevice()->CreateBuffer(&bd, nullptr, &m_matrixBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer 1");
		return;
	}
	//vertex shader constant buffer 2
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightBufferType);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_dx->GetDevice()->CreateBuffer(&bd, nullptr, &m_lightBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer 3");
		return;
	}
}

//render function sets all the buffers and shaders to render objects & shadows.		
void ShadowShader::Render(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection, XMMATRIX &lView, XMMATRIX &lProjection, ID3D11ShaderResourceView* texture, Light* l)
{
	XMMATRIX worldMatrix = world; //current object's world matrix
	XMMATRIX camView = view;	//camra view mtrix
	XMMATRIX camProjection = projection;	//camera projection
	XMMATRIX lightView = lView;		//light view matrix
	XMMATRIX lightProjection = lProjection;		//light projection matrix
	ID3D11ShaderResourceView* depthMapTexture = m_dx->GetDepthMap();		//depth map texture resource
	Light* light = l;  //light reference for light's position access

	MatrixBufferType cb; //vertex shader constant buffer 1 passes all the matrices
	//transpose matrices as shader uses different layout
	cb.world = XMMatrixTranspose(worldMatrix);
	cb.view = XMMatrixTranspose(camView);
	cb.projection = XMMatrixTranspose(camProjection);
	cb.lightView= XMMatrixTranspose(lightView);
	cb.lightProjection = XMMatrixTranspose(lightProjection);
	//update constant buffer
	m_dx->GetContext()->UpdateSubresource(m_matrixBuffer, 0, NULL, &cb, 0, 0);
	//send constant buffer to shader
	m_dx->GetContext()->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	//light's position buffer for vertex shader
	LightBufferType lb; 
	lb.lightPosition = light->GetPosition(); //get light's position
	lb.padding = 0.0f;		//padding can be any number
	//update buffer
	m_dx->GetContext()->UpdateSubresource(m_lightBuffer, 0, NULL, &lb, 0, 0);
	//send buffer to shader
	m_dx->GetContext()->VSSetConstantBuffers(1, 1, &m_lightBuffer);
	
	//passing two textures to shader
	m_dx->GetContext()->PSSetShaderResources(0, 1, &texture);
	m_dx->GetContext()->PSSetShaderResources(1, 1, &depthMapTexture);

	//passing sampler state to shader
	m_dx->GetContext()->PSSetSamplers(0, 1, &m_sampleStateClamp);

	// Set the input layout
	m_dx->GetContext()->IASetInputLayout(m_shadowShaderLayout);
	// Set the vertex and pixel shaders
	m_dx->GetContext()->VSSetShader(m_shadowVS, nullptr, 0);
	m_dx->GetContext()->PSSetShader(m_shadowPS, nullptr, 0);
	
	//draw call is issued by an object being rendered after shadow shader competes settings
}
