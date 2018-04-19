#include "stdafx.h"
#include "Sign.h"
//include texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"

//class that renders an arrow sprite
Sign::Sign(GraphicsEngine* ge)
{
	m_engine = ge; //reference to engine
}

Sign::~Sign()
{
	//release all rendering resources & null their pointers
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
	//null engine reference
	m_engine = nullptr;
}

#define DEF_SCALE (4.0f) //define default scale constant from which scaling will be calculated
#define NUM_VERTICES 6 // i need to render a plane made from 3 triangles = 6 vertices
// This function is called once at startup when the object is added to renderable list
void Sign::Initialise(DirectXWrapper *wrapper)
{
	//set variales to their default values
	m_scale = 1.0f; 
	//initialise all vectors to 0,0,0
	m_startPosition = Vector3(0,0,0);
	m_position = m_startPosition;
	m_targetPosition = m_startPosition;
	
	//initialising array of vertices 
	Vertex verts[NUM_VERTICES];
	//fill in vertices
	verts[0] = { Vector3(-1, 1, 0), Vector2(1, 0), Vector3(0, 1, 0) };
	verts[1] = { Vector3(1, 1, 0), Vector2(1, 1), Vector3(0, 1, 0) };
	verts[2] = { Vector3(-1, -1, 0), Vector2(0, 0), Vector3(0, 1, 0) }; //triangle 1
	verts[3] = { Vector3(-1, -1, 0), Vector2(0, 0), Vector3(0, 1, 0) };
	verts[4] = { Vector3(1, 1, 0), Vector2(1, 1), Vector3(0, 1, 0) };
	verts[5] = { Vector3(1, -1, 0), Vector2(0, 1), Vector3(0, 1, 0) }; //triangle 2

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
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex buffer");
		return;
	}

	//NOTE: most of object are using instanced drawing for a reasons explained in documentation
	//I only need one instance of this object so I create padding instane buffer with null position
	//creating padding instance data
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

	//load a texture
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Arrow.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load sign texture \n");
	}

	return;
}

// This function is called once per frame 
void Sign::Render(DirectXWrapper *wrapper, int pass)
{
	//setup matrices for transformation
	XMMATRIX scaling = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x,m_position.y,m_position.z);
	XMMATRIX rotationY = m_engine->GetCameraRotationY(); //rotation is set to always face camera on Y axis
	XMMATRIX worldTransform = scaling * rotationY *  translation;
	m_engine->SetWorldTransform(worldTransform);  //set world transform

	//set buffers
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(InstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = m_vertexBuffer;
	buffers[1] = m_instanceBuffer;

	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	//render using correct sending depending on pass
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
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, 1, 0, 0);

	return;

}

void Sign::Update()
{
	//calculate scale
	m_targetPosition = m_engine->GetPlayerPosition();

	float maxOffset = 34.0f; //offset is set to  mazesize (17 * 2)
	Vector3 difference = m_position - m_targetPosition; //difference in destination and player position
	//need to calculate how much of that distance has player arleady covered
	float value = ((difference.x / maxOffset) + (difference.z / maxOffset)) / 2; //average of both axis
	//calculate scale
	m_scale = DEF_SCALE * value;
	//clamp scale to min of 0.5
	if (m_scale <= 0.5f)
		m_scale = 0.5f;

	//set position
	m_position = m_startPosition;
	//adjust Y position
	m_position.y = m_startPosition.y + 2 * m_scale ; //4 is an offset 

	return;
}

void Sign::SetPosition(float x, float y, float z)
{
	m_startPosition = Vector3(x, y , z);

	return;
}



