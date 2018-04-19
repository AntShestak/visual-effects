# include "stdafx.h"
#include "Ramp.h"

#include "DirectXTK/Inc/WICTextureLoader.h"

Ramp::Ramp(GraphicsEngine* ge, Vector3 pos)
{
	m_engine = ge; //reference the engine
	//assigning mid position of the ramp for further calculation
	m_position = pos;
}

Ramp::~Ramp()
{
	//release rendering resources & null their pointers
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	//null engine pointer
	m_engine = nullptr;
}
//define the total number of vertices
#define NUM_VERTICES 6	//6 for two triangles

void Ramp::Initialise(DirectXWrapper *wrapper)
{
	Vertex verts[NUM_VERTICES]; //create array

	//now I nned to calculate 4 vertices using mid position which is actually in float2
	//where: one = 0,0    two = 0,1     three = 1,0		four 1,1
	Vector3 one = Vector3(m_position.x - 1, -1, m_position.z - 1);
	Vector3 two = Vector3(m_position.x - 1, -1, m_position.z + 1);
	Vector3 three = Vector3(m_position.x + 3, 1, m_position.z - 1);
	Vector3 four = Vector3(m_position.x + 3, 1, m_position.z + 1);

	//now from precalculated vertices I create two triangles, get the right uv's, andset normal to point up
	verts[0] = { three, Vector2(0, 0), Vector3(0, 1, 0) };	verts[1] = { one, Vector2(1, 0), Vector3(0, 1, 0) };	verts[2] = { four, Vector2(0, 1), Vector3(0, 1, 0) }; //triangle 1
	verts[3] = { four, Vector2(0, 1), Vector3(0, 1, 0) };	verts[4] = { one, Vector2(1, 0), Vector3(0, 1, 0) };	verts[5] = { two, Vector2(1, 1), Vector3(0, 1, 0) }; //triangle 2

	// creating vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(verts);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verts;
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	// m_vertexBuffer now points at an interface to a DirectX buffer object containing a copy of our vertices
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex buffer");
		return;
	}

	//because my shaders use intanced rendering i use this empty buffer for padding
	InstanceData padding;
	padding.position = Vector3(0.0f, 0.0f, 0.0f);
	//creating instance buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(InstanceData);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &padding;
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create instance buffer");
		return;
	};

	//loading the texture
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Ramp.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load ramp texture");
	}

}

// rendr function is called once per frame
void Ramp::Render(DirectXWrapper *wrapper, int pass)
{
	// Set the world transform to identity because vertices position is calculated already
	XMMATRIX worldTransform = XMMatrixIdentity();
	//send the world tansform to the engine
	m_engine->SetWorldTransform(worldTransform);

	//setup the vertex & instance buffers
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(InstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = m_vertexBuffer;
	buffers[1] = m_instanceBuffer;
	//send buffers to the device
	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	//pass 1 renders to depth texture , pass 2 renders to screen
	switch (pass)
	{
	case 1:
		m_engine->RenderShadowMap();
		break;
	case 2:
		m_engine->RenderToScreen(m_textureView);
		break;
	}

	// finally issue the draw call 
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, 1, 0, 0); //1 is for 1 instance
}