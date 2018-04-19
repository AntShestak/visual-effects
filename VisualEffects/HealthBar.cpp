#include "stdafx.h"
#include "HealthBar.h"

#include "DirectXTK/Inc/WICTextureLoader.h"

HealthBar::HealthBar(GraphicsEngine* ge, bool isBack)
{
	// nothing to do in the constructor
	m_engine = ge;
	m_isBackSide = isBack;
}

HealthBar::~HealthBar()
{
	//release all resources and null their pointers
	if (m_vertBuffer) m_vertBuffer->Release();
	m_vertBuffer = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
}

#define NUM_VERTICES 6 //define number of vertices

void HealthBar::Initialise(DirectXWrapper *wrapper)
{
	m_startPosition = Vector2(0.755f, 0.855f); //set position right top of the screen
	m_position = m_startPosition;
	m_startScale = Vector2(0.2f, 0.05f); //set appropriate scale
	m_scale = m_startScale;

	UVVertex verts[NUM_VERTICES]; //create array ( using UVVertex struct because no normals are needed

	//create two triangles to form a plane. and assigning UV's
	verts[0] = { Vector3(1, 1, 0), Vector2(1, 0) };	verts[1] = { Vector3(1, -1, 0), Vector2(1, 1) };	verts[2] = { Vector3(-1, 1, 0), Vector2(0, 0) }; //triangle 1
	verts[3] = { Vector3(-1, 1, 0), Vector2(0, 0) };	verts[4] = { Vector3(1, -1, 0), Vector2(1, 1) };	verts[5] = { Vector3(-1, -1, 0), Vector2(0, 1) }; //triangle 2

	//create vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = NUM_VERTICES*sizeof(UVVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verts;
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_vertBuffer);

	//if we are rendering back side set red texture and set depth
	if (m_isBackSide)
	{
		hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"BarRed.png", &m_texture, &m_textureView);
		if (FAILED(hr))
		{
			OutputDebugStringA("Failed to load health back texture \n");
		}

		m_depth = 0.1;
	}
	else //if it's the other( green bar) set green texture and depth
	{
		hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"BarGreen.png", &m_texture, &m_textureView);
		if (FAILED(hr))
		{
			OutputDebugStringA("Failed to load health back texture \n");
		}

		m_depth = 0;
	}

	return;
}

void HealthBar::Render(DirectXWrapper *wrapper, int pass) //doesnt matter the pass here
{
	//set transformation matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale.x, m_scale.y, 1);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_depth);
	XMMATRIX worldTransform =  scaling * translation ;
	m_engine->SetWorldTransform(worldTransform);
	

	// set the vertex buffer
	UINT stride = sizeof(UVVertex); 
	UINT offset = 0;
	wrapper->GetContext()->IASetVertexBuffers(0, 1, &m_vertBuffer, &stride, &offset);

	//call rendering function ( for UI objects)
	m_engine->RenderUI(m_textureView);

	// issue the draw call
	wrapper->GetContext()->Draw(NUM_VERTICES, 0);

	return;
}

void HealthBar::SetHealth(float percent)
{
	//as health get lower bar is scaled down ( front bar only) only on X axis
	m_scale.x = m_startScale.x * percent;
	//but after scaling it has to be moved ( otherwise it stays positioned in the middle)
	m_position.x = m_startPosition.x + (m_startScale.x * (1 - percent)) ;
	
	return;
}