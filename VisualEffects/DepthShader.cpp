#include "stdafx.h"
#include "DepthShader.h"
//include compiled shader files
#include "CompiledDepthVS.h"
#include "CompiledDepthPS.h"

//constant buffer
struct MatrixBufferType
{
	XMMATRIX world; //objects world matrix
	XMMATRIX lightView;			//light's view
	XMMATRIX lightProjection;	//light's projection
};

DepthShader::DepthShader(DirectXWrapper *dx) //constructor
{
	m_dx = dx;		//directX reference
	Initialise();	//call initialise
}

DepthShader::~DepthShader()
{
	//realease all the resources
	if (m_depthVS) m_depthVS->Release();
	if (m_depthShaderLayout)m_depthShaderLayout->Release();
	if (m_depthPS) m_depthPS->Release();
	if (m_matrixBuffer)m_matrixBuffer->Release();
}

void DepthShader::Initialise()
{
	// creating vertex shader from precompiled file
	HRESULT hr = m_dx->GetDevice()->CreateVertexShader(g_CompiledDepthVS, sizeof(g_CompiledDepthVS), nullptr, &m_depthVS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex shader");
		return;
	}

	// create ixel shader
	hr = m_dx->GetDevice()->CreatePixelShader(g_CompiledDepthPS, sizeof(g_CompiledDepthPS), nullptr, &m_depthPS);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create pixel shader");
		return;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	//objects position
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },		//instance positions
	};
	UINT numElements = ARRAYSIZE(layout);
	// Create the input layout
	hr = m_dx->GetDevice()->CreateInputLayout(layout, numElements, g_CompiledDepthVS, sizeof(g_CompiledDepthVS), &m_depthShaderLayout);
	
	//set the buffer properties
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatrixBufferType);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	//create costant buffer
	hr = m_dx->GetDevice()->CreateBuffer(&bd, nullptr, &m_matrixBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer");
		return;
	}

}

void DepthShader::Render(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)
{
	XMMATRIX worldMatrix = world; //object to render world matrix
	XMMATRIX lightView = view;	//light view matrix
	XMMATRIX lightProjection = projection;	//light projection matrix

	//setting constant buffer
	MatrixBufferType cb;
	//transpose matrices as shaders use different matrix layout
	cb.world = XMMatrixTranspose(worldMatrix);
	cb.lightView = XMMatrixTranspose(view);
	cb.lightProjection = XMMatrixTranspose(projection);
	//updating buffer
	m_dx->GetContext()->UpdateSubresource(m_matrixBuffer, 0, NULL, &cb, 0, 0);
	//send it to shader
	m_dx->GetContext()->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Set the input layout
	m_dx->GetContext()->IASetInputLayout(m_depthShaderLayout);
	// Set the vertex and pixel shaders
	m_dx->GetContext()->VSSetShader(m_depthVS, nullptr, 0);
	m_dx->GetContext()->PSSetShader(m_depthPS, nullptr, 0);

	//draw call is issued by object being rendered after depth shader setting is complete
}
