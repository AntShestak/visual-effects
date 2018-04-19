#include "stdafx.h"
#include "Collectible.h"
//include texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"

Collectible::Collectible(GraphicsEngine* ge)
{
	m_engine = ge; //set refernce to the engine
}

Collectible::~Collectible() //destructor
{
	//release rendering resources & null their pointers
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;

	m_engine = nullptr;  //null engine pointer
}
void Collectible::Destroy()
{
	m_isActive = false; //indicates that object is not active, later parent object will delete it
	return;
}

#define NUM_VERTICES 36 //define the nuber of vertices 6 faces = 12 triangles = 36 vertices

// This function is called once when object is added to renderable list
void Collectible::Initialise(DirectXWrapper *wrapper)
{
	Vertex verts[NUM_VERTICES]; 

	//Assigning verts array values manually
	//I'm going to map UV's manually because "principal axes doesn't work with my cube", so I use the same method manually
	//is is easy to determine the principal axis on the side of cube, as it is the only coordinate that doesn't change on 6 vertices
	//then what is -1, becomes 0 on UV, what is 1 becomes 1.

	//Side 1 , consists of 2 triangles as any other side
	verts[0].position = Vector3(-1, 1, -1);		verts[0].uv = Vector2(0, 0);
	verts[1].position = Vector3(1, 1, -1);		verts[1].uv = Vector2(1, 0);
	verts[2].position = Vector3(-1, -1, -1);	verts[2].uv = Vector2(0, 1);//triangle 1
	verts[3].position = Vector3(-1, -1, -1);	verts[3].uv = Vector2(0, 1);
	verts[4].position = Vector3(1, 1, -1);		verts[4].uv = Vector2(1, 0);
	verts[5].position = Vector3(1, -1, -1);		verts[5].uv = Vector2(1, 1);//triangle 2
	//normal will be the same for all vertices of the same faces
	Vector3 norm = CalculateNormal(verts[0].position, verts[1].position, verts[2].position);
	verts[0].normal = norm;	verts[1].normal = norm; verts[2].normal = norm;
	verts[3].normal = norm; verts[4].normal = norm; verts[5].normal = norm;

	//Side 2 , consists of 2 triangles as any other side
	verts[6].position = Vector3(1, 1, -1);		verts[6].uv = Vector2(0, 0);
	verts[7].position = Vector3(1, 1, 1);		verts[7].uv = Vector2(1, 0);
	verts[8].position = Vector3(1, -1, -1);		verts[8].uv = Vector2(0, 1);//triangle 3
	verts[9].position = Vector3(1, 1, 1);		verts[9].uv = Vector2(1, 0);
	verts[10].position = Vector3(1, -1, 1);		verts[10].uv = Vector2(1, 1);
	verts[11].position = Vector3(1, -1, -1);	verts[11].uv = Vector2(0, 1);//triangle 4
	//normal will be the same for all vertices of the same faces
	norm = CalculateNormal(verts[6].position, verts[7].position, verts[8].position);
	verts[6].normal = norm;	verts[7].normal = norm; verts[8].normal = norm;
	verts[9].normal = norm; verts[10].normal = norm; verts[11].normal = norm;

	//Side 3 , consists of 2 triangles as any other side
	verts[12].position = Vector3(1, -1, 1);		verts[12].uv = Vector2(0, 1);
	verts[13].position = Vector3(1, 1, 1);		verts[13].uv = Vector2(0, 0);
	verts[14].position = Vector3(-1, -1, 1);	verts[14].uv = Vector2(1, 1);//triangle 5
	verts[15].position = Vector3(-1, -1, 1);	verts[15].uv = Vector2(1, 1);
	verts[16].position = Vector3(1, 1, 1);		verts[16].uv = Vector2(0, 0);
	verts[17].position = Vector3(-1, 1, 1);		verts[17].uv = Vector2(1, 0);//triangle 6
	//normal will be the same for all vertices of the same faces
	norm = CalculateNormal(verts[12].position, verts[13].position, verts[14].position);
	verts[12].normal = norm;	verts[13].normal = norm; verts[14].normal = norm;
	verts[15].normal = norm; verts[16].normal = norm; verts[17].normal = norm;

	//Side 4 , consists of 2 triangles as any other side
	verts[18].position = Vector3(-1, 1, 1);		verts[18].uv = Vector2(0, 0);
	verts[19].position = Vector3(-1, 1, -1);	verts[19].uv = Vector2(1, 0);
	verts[20].position = Vector3(-1, -1, 1);	verts[20].uv = Vector2(0, 1);//triangle 7
	verts[21].position = Vector3(-1, -1, 1);	verts[21].uv = Vector2(0, 1);
	verts[22].position = Vector3(-1, 1, -1);	verts[22].uv = Vector2(1, 0);
	verts[23].position = Vector3(-1, -1, -1);	verts[23].uv = Vector2(1, 1);//triangle 8
	//normal will be the same for all vertices of the same faces
	norm = CalculateNormal(verts[18].position, verts[19].position, verts[20].position);
	verts[18].normal = norm;	verts[19].normal = norm; verts[20].normal = norm;
	verts[21].normal = norm; verts[22].normal = norm; verts[23].normal = norm;

	//Side 5 , consists of 2 triangles as any other side
	verts[24].position = Vector3(-1, 1, -1);	verts[24].uv = Vector2(0, 0);
	verts[25].position = Vector3(-1, 1, 1);		verts[25].uv = Vector2(1, 0);
	verts[26].position = Vector3(1, 1, -1);		verts[26].uv = Vector2(0, 1);//triangle 9
	verts[27].position = Vector3(1, 1, -1);		verts[27].uv = Vector2(0, 1);
	verts[28].position = Vector3(-1, 1, 1);		verts[28].uv = Vector2(1, 0);
	verts[29].position = Vector3(1, 1, 1);		verts[29].uv = Vector2(1, 1);//triangle 10
	//normal will be the same for all vertices of the same faces
	norm = CalculateNormal(verts[24].position, verts[25].position, verts[26].position);
	verts[24].normal = norm;	verts[25].normal = norm; verts[26].normal = norm;
	verts[27].normal = norm; verts[28].normal = norm; verts[29].normal = norm;

	//Side 6 , consists of 2 triangles as any other side
	verts[30].position = Vector3(-1, -1, 1);	verts[30].uv = Vector2(0, 0);
	verts[31].position = Vector3(-1, -1, -1);	verts[31].uv = Vector2(1, 0);
	verts[32].position = Vector3(1, -1, -1);	verts[32].uv = Vector2(1, 1);//triangle 11
	verts[33].position = Vector3(-1, -1, 1);	verts[33].uv = Vector2(0, 0);
	verts[34].position = Vector3(1, -1, -1);	verts[34].uv = Vector2(1, 1);
	verts[35].position = Vector3(1, -1, 1);		verts[35].uv = Vector2(0, 1);//triangle 12
	//normal will be the same for all vertices of the same faces
	norm = CalculateNormal(verts[30].position, verts[31].position, verts[32].position);
	verts[30].normal = norm;	verts[31].normal = norm; verts[32].normal = norm;
	verts[33].normal = norm; verts[34].normal = norm; verts[35].normal = norm;

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
	};

	//my shaders are to use nstanced rendering
	//so I have to create an empty padding buffer
	InstanceData padding;
	padding.position = Vector3(0.0f, 0.0f, 0.0f);
	//create instance buffer
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
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"HealthPickUp.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load Collectible texture");
	}

	//initialising variables
	m_angle = 0;
	m_isActive = true;
}

void Collectible::Render(DirectXWrapper *wrapper, int pass)
{
	XMMATRIX translation;
	if (m_isActive) //if pickUp is Active I use  normal matrix with its m_position
		translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	else //if it isn't active I position it very low below surface (so it's not seen);
		translation = XMMatrixTranslation(0.0f, -100.0f, 0.0f);
	//set rotation & scale
	XMMATRIX scale = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX rotation = XMMatrixRotationY(m_angle);
	XMMATRIX worldTransform = scale * rotation * translation;
	//update transform matrix
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

	//depending on pass variable use apropriate render function
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
}

void Collectible::SetPosition(Vector3 pos)
{
	m_position = pos;
}

void Collectible::SetScale(float scale)
{
	m_scale = scale;
}

void Collectible::Update(float dTime)
{
	m_angle += dTime;	//adding dTime this gives normal rottion speed
	if (m_angle > 360.0f)
		m_angle -= 360.0f; //so we don't get to huge numbers
}

//calculates rmal given 3 vertices of the face
Vector3 Collectible::CalculateNormal(Vector3 &vert1, Vector3 &vert2, Vector3 &vert3)
{
	//gt two vectors
	Vector3 vec1 = vert2 - vert1;
	Vector3 vec2 = vert3 - vert2;
	//get that cross product from these two vectors
	Vector3 nor = vec1.Cross(vec2);
	nor.Normalize();//normalise cross 
	
	return nor; //return a normal
}