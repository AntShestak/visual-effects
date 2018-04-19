#include "stdafx.h"
#include "GameEndBoard.h"

#include "DirectXTK/Inc/WICTextureLoader.h"

GameEndBoard::GameEndBoard(GraphicsEngine* ge)
{
	m_engine = ge; //assign engine reference
}

GameEndBoard::~GameEndBoard()
{
	//release rendering resources
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_texture) m_texture->Release();
	if (m_textureView) m_textureView->Release();
}

#define NUM_VERTICES 6

//this function is called when the object is added to renderabl object list
void GameEndBoard::Initialise(DirectXWrapper *wrapper)
{
	UVVertex verts[NUM_VERTICES]; //create an array of UVVertex structure, no need for normal as UI is rendered in 2D style

	//create two triangles with correct UV's
	verts[0] = { Vector3(1, 1, 0), Vector2(1, 0) };	verts[1] = { Vector3(1, -1, 0), Vector2(1, 1) };	verts[2] = { Vector3(-1, 1, 0), Vector2(0, 0) }; //triangle 1
	verts[3] = { Vector3(-1, 1, 0), Vector2(0, 0) };	verts[4] = { Vector3(1, -1, 0), Vector2(1, 1) };	verts[5] = { Vector3(-1, -1, 0), Vector2(0, 1) }; //triangle 2

	// create vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = NUM_VERTICES*sizeof(UVVertex);
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
	//load texture
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"GameWon.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load GameEndBoard texture \n");
	}

}

// This function is called once per frame 
void GameEndBoard::Render(DirectXWrapper *wrapper, int pass)
{
	//set the translation & scaling matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX worldTransform = scaling *   translation;
	m_engine->SetWorldTransform(worldTransform); //update world transform with the new matrix

	//set the vertex buffer
	UINT stride = sizeof(UVVertex); 
	UINT offset = 0; 
	wrapper->GetContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	m_engine->RenderUI(m_textureView); //render using special UI rendering ( pass the texture)

	// issue the draw call 
	wrapper->GetContext()->Draw(NUM_VERTICES, 0);

	return;
}

void GameEndBoard::SetPosition(float x, float y, float z) //sets position of the object
{
	m_position = Vector3(x, y, z);
}

void GameEndBoard::SetScale(float s) //sets scale
{
	m_scale = s;
}


