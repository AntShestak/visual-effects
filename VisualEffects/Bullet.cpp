#include "stdafx.h"
#include "Bullet.h"

#include "DirectXTK/Inc/WICTextureLoader.h"


//constructor sets the reference to an engine object
Bullet::Bullet(GraphicsEngine* ge)
{
	m_engine = ge;
}

Bullet::~Bullet() //destructor
{
	//release all the rendering resources & set pointers to null
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	//fire particle system
	m_engine->RemoveParticleObject(m_fire); //remove from rendering list
	delete m_fire; //delete &null pointer
	m_fire = nullptr;

	m_engine = nullptr; //null engine pointer
}

//sphere generation
//adds a triangle to the list
void Bullet::PushBackTriangle(Vertex &vert1, Vertex &vert2, Vertex &vert3)
{
	m_listFaces.push_back(vert1);
	m_listFaces.push_back(vert2);
	m_listFaces.push_back(vert3);
}


//retuns a vertex that lies in-between two vertices passed into this function
Vertex Bullet::GetMidPoint(Vertex &vert1, Vertex &vert2)
{
	Vertex ret;
	// calculate mid point
	ret.position = (vert1.position + vert2.position) / 2.0f;
	// normalise it 
	ret.position.Normalize();

	return ret;
}

#define MAX_SUBDIVISION 1 //bullet is covered with fire particles so doesn't need deep subdividing
//function that subdivides a face passed into the function
void Bullet::Subdivide(Vertex &vert1, Vertex &vert2, Vertex &vert3, int depth)
{
	// first calculate mid point for each of the three edges of the triangle passed
	Vertex vert4 = GetMidPoint(vert1, vert2);
	Vertex vert5 = GetMidPoint(vert2, vert3);
	Vertex vert6 = GetMidPoint(vert3, vert1);
	//if to these are connected correctly, I get 4 triangles out of one passed

	// if MAX_SUBDIVISION is reached add the faces to the list of triangles
	if (depth == MAX_SUBDIVISION)
	{
		PushBackTriangle(vert1, vert4, vert6);
		PushBackTriangle(vert4, vert2, vert5);
		PushBackTriangle(vert5, vert3, vert6);
		PushBackTriangle(vert4, vert5, vert6);
	}
	else //if MAX_SUBDIVISION is not reached yet have to subdivide more before adding faces
	{
		//subdivide each of the 4 triangles created by mid point calculation, and increase depth to keep track of subdivision level
		Subdivide(vert1, vert4, vert6, depth + 1);
		Subdivide(vert4, vert2, vert5, depth + 1);
		Subdivide(vert5, vert3, vert6, depth + 1);
		Subdivide(vert4, vert5, vert6, depth + 1);
	}

	return;
}


// This function is called once at startup when the object is added to DirectXWrapper
void Bullet::Initialise(DirectXWrapper *wrapper)
{

	// as a base of a sphere is used polyhedron with 8 faces and 6 vertices
	//temporarily setting normals & UV's to zero's
	Vertex octahedron[6] = { { { 0, 1, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { -1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, 1 }, { 0, 0 }, { 0, 0, 0 } }, { { 1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, -1 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, -1, 0 }, { 0, 0 }, { 0, 0, 0 } } };
	// we recursively subdivide an octahedron
	Subdivide(octahedron[0], octahedron[1], octahedron[2], 0);
	Subdivide(octahedron[0], octahedron[2], octahedron[3], 0);
	Subdivide(octahedron[0], octahedron[3], octahedron[4], 0);
	Subdivide(octahedron[0], octahedron[4], octahedron[1], 0);
	Subdivide(octahedron[5], octahedron[2], octahedron[1], 0);
	Subdivide(octahedron[5], octahedron[3], octahedron[2], 0);
	Subdivide(octahedron[5], octahedron[4], octahedron[3], 0);
	Subdivide(octahedron[5], octahedron[1], octahedron[4], 0);

	//mapping normals
	for (unsigned int i = 0; i < m_listFaces.size(); i++)
	{
		//normals first, because there are required for UV mapping
		Vector3 norm = m_listFaces[i].position; //normals equals to position, because our sphere is unit sphere, so position is a vector from center to surface point
		norm.Normalize(); //normalie it anyway
		m_listFaces[i].normal = norm;
		//now map UV
		m_listFaces[i].uv.x = m_listFaces[i].normal.x / 2 + 0.5f; //x in a range from 0 to 1, where norm.x = -1, u = 0, where norm.x = 1 , u = 1
		m_listFaces[i].uv.y = (m_listFaces[i].normal.y * -1) / 2 + 0.5f; //have to multiply by -1, because V's start from top, so where Y is 1, V has to be 0
	}

	// create vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = m_listFaces.size()*sizeof(Vertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_listFaces[0];
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex buffer");
		return;
	}

	//shader is set to use instanced drawing so I have to create a buffer for padding with 0,0,0 position
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
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Lava.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load enemy texture");
	}
	//set the member variables
	m_scale = 0.2f;
	m_speed = 1.8f; //slightly faster than enemy
	m_lifeSpan = 10.0f;
	m_isActive = true;
	m_timeAlive = 0;
	//start a fire
	StartFire();

	return;
}

//render is called from DirectXWrapper for each object in renderable list every frame
void Bullet::Render(DirectXWrapper *wrapper, int pass)
{
	//set the transformation matrices
	XMMATRIX scale = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translationMatrix = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX worldTransform = scale * translationMatrix;
	//update the world transform
	m_engine->SetWorldTransform(worldTransform);

	//set the buffers
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

	//depending on rendering pass , render using apropriate shaders
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
	wrapper->GetContext()->DrawInstanced(m_listFaces.size(), 1, 0, 0); //1 is for 1 instance
}

void Bullet::StartFire() //function that sets up a particle system
{
	float scale = 0.3f; //size of the sphere that particles spawn around
	m_fire = new FireSphere(m_engine); //create a particle system
	m_engine->AddParticleObject(m_fire); //add to the renderable object list
	m_fire->SetProperties(scale, 0.3f, 600, 2, 0.7f, m_speed); //set system properties. NOTE: 1st parameter is explained above, but 2nd parameter(scale) is actual scale of particle object
	m_fire->SetPosition(m_position); //set position
}

void Bullet::SetStartPosition(Vector3 pos, Vector2 dir) 
{
	m_position = pos; //set position
	m_direction = dir;	//set movement direction
}

void Bullet::Destroy() //called when bullet collides with something
{
	m_isActive = false; //simply deactivate the bullet, it will be cleaned up by it's parent
	return;
}

void Bullet::Update(float dTime) 
{
	m_timeAlive += dTime; //add to time alive
	//check if bullet life is not over
	if (m_timeAlive >= m_lifeSpan)
		m_isActive = false;
	
	//move bullet
	m_position.x += m_direction.x * m_speed * dTime;
	m_position.z += m_direction.y * m_speed * dTime;

	//update particles
	m_fire->SetPosition(m_position); //update position
	Vector3 dir = Vector3(m_direction.x, 0.0f, m_direction.y); //tranform direction into vector3
	m_fire->Update(dTime, dir); //update particle system

	//check if bullet has collided with wall
	if (m_engine->CheckWallCollision(m_position.x, m_position.z, 0.1f, 2))
		m_isActive = false; //if has collided deactivate

	return;
		
}
