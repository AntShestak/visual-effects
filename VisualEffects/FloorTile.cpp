#include "stdafx.h"
#include "FloorTile.h"
//include texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"


//constructor
FloorTile::FloorTile(GraphicsEngine* ge, int stage)
{
	m_engine = ge;	//reference the engine
	m_levelNum = stage;	//assign level number
}

FloorTile::~FloorTile()
{
	//release rendering resources
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
}

#define NUM_VERTICES 6 //floor tile is a square so I need 2 triangles = 6 vertices
// This function is called once when object is added to renderable list
void FloorTile::Initialise(DirectXWrapper *wrapper)
{
	//set main tile properties to default
	m_position = Vector3(0, 0, 0);
	m_scale = 1.0f;
	
	//initialising array of vertices
	Vertex verts[NUM_VERTICES];
	//filling each vertex with values
	//all normals point up as it is a floor
	verts[0] = { Vector3(-1, 0, 1),Vector2(0 , 1), Vector3(0, 1, 0)};	
	verts[1] = { Vector3(1, 0, 1), Vector2(1 , 1), Vector3(0, 1, 0)};	
	verts[2] = { Vector3(-1, 0, -1), Vector2(0, 0), Vector3(0, 1, 0) }; //triangle 1
	verts[3] = { Vector3(-1, 0, -1), Vector2(0, 0), Vector3(0, 1, 0) };
	verts[4] = { Vector3(1, 0, 1), Vector2(1, 1), Vector3(0, 1, 0) };
	verts[5] = { Vector3(1, 0, -1), Vector2(1, 0), Vector3(0, 1, 0) }; //triangle 2

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

	//creating instance buffer
	/*
		NOTE: must fill list of instances before calling this function ( therefore before passing Floortile to Renderable list )
	*/
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = m_listInstances.size() * sizeof(InstanceData);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_listInstances[0];
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create instance buffer");
		return;
	};

	//loading the texture
	if (m_levelNum == 1)
	{
		//if it is level 1 - load level 1 texture )
		hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"StoneTile.jpg", &m_texture, &m_textureView);
		if (FAILED(hr))
		{
			OutputDebugStringA("Failed to load floor texture \n");
		}
	}
	else
	{
		//if it is level 2 - load level 2 texture )
		hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Path.jpg", &m_texture, &m_textureView);
		if (FAILED(hr))
		{
			OutputDebugStringA("Failed to load floor texture \n");
		}
	}
	return;
}

// This function is called once per frame for every renderable object in the list
void FloorTile::Render(DirectXWrapper *wrapper,int pass)
{
	//set transformation matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale, 1, m_scale);
	XMMATRIX translation = XMMatrixIdentity(); //translation is performed in verex shader , so used base translation
	XMMATRIX worldTransform = scaling * translation;
	//update matrices
	m_engine->SetWorldTransform(worldTransform);

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
	
	//depending on PASS render with correct settings
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
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, m_listInstances.size(), 0, 0);

	return;
}

void FloorTile::AddInstance(Vector3 pos) //adds an instance position to the list
{
	InstanceData newInstance; //create new instance
	newInstance.position = pos; 
	m_listInstances.push_back(newInstance); //add to the list

	return;
}
//sets object scale
void FloorTile::SetScale(float s)
{
	m_scale = s;

	return;
}


