#include "stdafx.h"
#include "Squasher.h"
//loading textures using DirectXTK 
#include "DirectXTK/Inc/WICTextureLoader.h"

//constructor
Squasher::Squasher(GraphicsEngine* ge, MazeGenerator* maze)
{
	m_engine = ge;
	m_maze = maze; //maze is used for Dijkstra
}
//dstructor
Squasher::~Squasher()
{
	//release COM objects
	if (pVertexBuffer) pVertexBuffer->Release();
	pVertexBuffer = nullptr;

	if (pInstanceBuffer) pInstanceBuffer->Release();
	pInstanceBuffer = nullptr;

	if (pTexture) pTexture->Release();
	pTexture = nullptr;

	if (pTextureView) pTextureView->Release();
	pTextureView = nullptr;
	//null the references
	m_engine = nullptr;
	m_maze = nullptr;
}
//adds three vertices (that make a triangle) to the list of faces
void Squasher::PushBackTriangle(Vertex &vert1, Vertex &vert2, Vertex &vert3)
{
	m_listFaces.push_back(vert1);
	m_listFaces.push_back(vert2);
	m_listFaces.push_back(vert3);
}
//retuns a vertex that lies in-between two vertices passed into this function
Vertex Squasher::GetMidPoint(Vertex &vert1, Vertex &vert2)
{
	Vertex ret;
	// calculate mid point
	ret.position = (vert1.position + vert2.position) / 2.0f;
	// normalise it 
	ret.position.Normalize();

	return ret;
}

#define MAX_SUBDIVISION 3	//level of subdivision. 3 is enough to make Squasher appear smooth
//function that subdivides a face given
void Squasher::Subdivide(Vertex &vert1, Vertex &vert2, Vertex &vert3, int depth)
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
}

void Squasher::Initialise(DirectXWrapper *wrapper)
{
	// as a base of a sphere is used polyhedron with 8 faces and 6 vertices
	//temporarily setting normals & UV's to zero's
	Vertex octahedron[6] = { { { 0, 1, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { -1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, 1 }, { 0, 0 }, { 0, 0, 0 } }, { { 1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, -1 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, -1, 0 }, { 0, 0 }, { 0, 0, 0 } } };
	//subivide each of the octahedron sides
	Subdivide(octahedron[0], octahedron[1], octahedron[2], 0);
	Subdivide(octahedron[0], octahedron[2], octahedron[3], 0);
	Subdivide(octahedron[0], octahedron[3], octahedron[4], 0);
	Subdivide(octahedron[0], octahedron[4], octahedron[1], 0);
	Subdivide(octahedron[5], octahedron[2], octahedron[1], 0);
	Subdivide(octahedron[5], octahedron[3], octahedron[2], 0);
	Subdivide(octahedron[5], octahedron[4], octahedron[3], 0);
	Subdivide(octahedron[5], octahedron[1], octahedron[4], 0);

	//when list of faces is complete can map normals & uv's
	//mapping normals && UV's
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

	//create a vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = m_listFaces.size()*sizeof(Vertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_listFaces[0];
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &pVertexBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex buffer");
		return;
	}

	//most of object are using instanced drawing for a reasons explained in documentation
	//I only need one instance of this object so I create padding intane buffer with null position
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
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &pInstanceBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create instance buffer");
		return;
	};

	//loading the texture
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Pool13.png", &pTexture, &pTextureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load enemy texture");
	}

	ResetPatrolPoints(); //reseting patrol points to start route finding
	m_rotationAxis = Vector3(0, 0, 0); //axis of rotation variable
	m_angle = 0;	//angle to rotate

}

void Squasher::Render(DirectXWrapper *wrapper,int pass)
{
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX scale = XMMatrixScaling(m_scale,m_scale,m_scale); 
	XMMATRIX rotation = XMMatrixRotationAxis(m_rotationAxis, m_angle);
	XMMATRIX worldTransform = scale * rotation * translation;
	m_engine->SetWorldTransform(worldTransform);

	// Point DirectX at the buffer's
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(InstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = pVertexBuffer;
	buffers[1] = pInstanceBuffer;
	//setting buffers
	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	//depending on pass number, m_engine will render this using apropriate shader class
	switch (pass)
	{
	case 1:
		m_engine->RenderShadowMap(); //in the first pass render to shadow map
		break;
	case 2:
		m_engine->RenderToScreen(pTextureView);	//in the 2nd pass render to backbuffer, texture needed here
		break;
	}
	// finally issue the draw call 
	wrapper->GetContext()->DrawInstanced(m_listFaces.size(), 1, 0, 0); //only one instance
}
//set position 
void Squasher::SetPosition(float x, float y, float z)
{
	m_position = Vector3(x, y, z);
}
//sets scale
void Squasher::SetScale(float s)
{
	m_scale = s;
}
//sets rotation, around axis passed , called every frame
void Squasher::UpdateRotation(Vector3 rotation, float deltaTime)
{
	m_rotationAxis = rotation; //axis to rotate around
	m_angle += 5 * deltaTime;	//increase angle every frame
	if (m_angle > 360.0f)	//angle could grow into huge number eventually
		m_angle -= 360.0f;
}

//AI logic
#define NUM_PATROL_POINTS 4 //if too few patrol points level could be uncompletable, due to randomised nature of patrol point picking
#define MOVEMENT_SPEED (2.0f) //adjusted so the enemy is a little bit slower than player

void Squasher::ResetPatrolPoints()
{
	// clear the patrol points list
	m_patrolPoints.clear();
	// generate a new list
	for (int i = 0; i < NUM_PATROL_POINTS; i++)
	{
		//get a random tile form the maze , and check if it' valid to be patrol point
		IntVec2 candidate;
		do
		{
			candidate = IntVec2(rand() % m_maze->GetWidth(), rand() % m_maze->GetHeight()); // random x,y
		} while (m_maze->IsSolid(candidate)); // this will loop until candidate is suitable
		//add suitable candidate to patrol point list
		m_patrolPoints.push_back(candidate);
	}
	// when list is generated enemy can start patrol
	m_nextPatrolPoint = 0; //next patrol point is the first on the list
	PlanRouteToNextPatrolPoint();	//plan route to it
}

//this function calculates a route to the next patrol point using Dijkstra and maze reference
void Squasher::PlanRouteToNextPatrolPoint()
{
	// if we are planning route to the next patrol point then currently we are at the previous patrol point
	IntVec2 startPatrol = m_patrolPoints[m_nextPatrolPoint];
	// get next patrol point index
	m_nextPatrolPoint++;
	// if we already went through all patrol point , head to first one
	if (m_nextPatrolPoint >= NUM_PATROL_POINTS)
		m_nextPatrolPoint = 0;
	//set the end patrol variable
	IntVec2 endPatrol = m_patrolPoints[m_nextPatrolPoint];
	// m_route is passed as a reference so it will be updated with a list of tiles from startPatrol tile to endPatrol tile
	m_maze->RunDijkstra(startPatrol, endPatrol, m_route);
	// set the current maze position to the first position in route
	m_posTile = m_route[0];
	// reset routeIndex to the first point in the list
	m_routeIndex = 0;
	// clear the movement vector and moveTarget variable
	m_moveVec = IntVec2(0, 0);
	m_moveTarget = m_posTile; // this will force an update
	m_posVec2 = ConvertToVector2(m_posTile); //converting position into real world units for enemy transformation
}
//update functions moves & rotates enemy object
void Squasher::Update(float dTime) 
{
	// start by checking if we reached our next route position
	if (m_posTile == m_moveTarget) //if target route tile is reached
	{
		// increment routeIndex 
		m_routeIndex++;
		if (m_routeIndex >= m_route.size()) //if we went over the route size
		{
			//plan new route ( to the next patrol point)
			PlanRouteToNextPatrolPoint();
			m_routeIndex = 1; //set index to seconds tile in the route, as we are already on the 1st one
		}
		// now we need to figure out where we're going and what movement vector we need to get there
		IntVec2 nextTile = m_route[m_routeIndex];
		//to calculae movement vector simply deduct the current tile from destination tile
		m_moveVec = nextTile - m_posTile;
		// update moveTarget
		m_moveTarget = nextTile;
	}
	// okay now we need to move in moveVec direction till we hit moveTarget
	//we know the movement vector , but we don't want to jump to the next route node
	//therefore here I start to do more complicated calculation using real world units as well as maze integer units
	Vector2 move = ConvertToVector2(m_moveVec); //converting movement vector from maze int units to world units
	move *= MOVEMENT_SPEED * dTime; //applying movement speed multiplied by delta time
	m_posVec2 += move; // adjusting position in world units

	Vector2 pos = ConvertToVector2(m_posTile); //here I get tile os in world units for calculations
	if (fabs(m_posVec2.x - pos.x) >= 2 || fabs(m_posVec2.y - pos.y) >= 2) // checking if we have got 1 maze unit far
	{
		m_posTile = m_moveTarget;	//updating maze position, as we moved one tile further
		m_posVec2 = ConvertToVector2(m_posTile);	//resetting world position to match maze position to exactly on the new tile
	}
	//setting enemy position
	SetPosition(m_posVec2.x, 0.0f, m_posVec2.y);
	//setting rotation
	UpdateRotation(Vector3(move.y, 0.0f, -move.x), dTime);

	return;
}
//helper function that converts from maze units to world units
Vector2 Squasher::ConvertToVector2(IntVec2 vec)
{
	float size = m_maze->GetTileSize(); //obtain the size of maze tile
	Vector2 ret = Vector2((float)vec.x * size, (float)vec.y * size);  //multiply maze position by tile size and transfer to floats
	
	return ret; 
}