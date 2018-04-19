 #include "stdafx.h"
#include "Player.h"

#include "DirectXTK/Inc/WICTextureLoader.h"

//this class renders a player object and takes care of it's movements and other player relevant actions
Player::Player(GraphicsEngine* ge)
{
	m_engine = ge;	//reference graphics engine
	cam = m_engine->GetCamera(); //get camera reference
	SetPosition(0, 0, 0);	//set position to default value

	m_input = new Input(); //create input class
}

Player::~Player()
{
	//elete rendering references and null their pointers
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	//delete input class 
	delete m_input;
	m_input = nullptr;
	//deleting paticle system sparks
	delete m_sparks;
	m_sparks = nullptr;
	//delete explosion particle system
	delete m_explosion;
	m_explosion = nullptr;
	//null references
	m_engine = nullptr;
	cam = nullptr;
	
}

//this function adds a three vertices to the list ( 3 vertices represent a triangle )
void Player::PushBackTriangle(Vertex &vert1, Vertex &vert2, Vertex &vert3)
{
	//add them to the list
	m_listFaces.push_back(vert1);
	m_listFaces.push_back(vert2);
	m_listFaces.push_back(vert3);

	return;
}
//function returns a middle point between two points
Vertex Player::GetMidPoint(Vertex &vert1, Vertex &vert2)
{
	Vertex ret;
	// calculate mid point
	ret.position = (vert1.position + vert2.position) / 2.0f;
	// normalise it 
	ret.position.Normalize();

	return ret;
}

#define MAX_SUBDIVISION 3 //three is enough to make player appear smooth

void Player::Subdivide(Vertex &vert1, Vertex &vert2, Vertex &vert3, int depth)
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

//this function is called when object is added to renderable list
void Player::Initialise(DirectXWrapper *wrapper)
{
	// as a base of a sphere is used polyhedron with 8 faces and 6 vertices
	//temporarily setting normals & UV's to zero's
	Vertex octahedron[6] = { { { 0, 1, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { -1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, 1 }, { 0, 0 }, { 0, 0, 0 } }, { { 1, 0, 0 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, 0, -1 }, { 0, 0 }, { 0, 0, 0 } }, { { 0, -1, 0 }, { 0, 0 }, { 0, 0, 0 } } };
	//subivide each of the octahedron sides
	// we recursively subdivide an octahedron
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
	for (unsigned int i = 0; i < m_listFaces.size(); i++) //iterate throught the list of vertices
	{
		//normals first, because they are required for UV mapping
		Vector3 norm = m_listFaces[i].position; //normal equals to position, because our sphere is unit sphere, so position is a vector from center to surface point
		norm.Normalize();
		m_listFaces[i].normal = norm;
		//now map UV
		//i want my texture to cover top of player so I work with X and Z properties of normal
		m_listFaces[i].uv.x = m_listFaces[i].normal.x / 2 + 0.5f; // normal is in range of -1 to 1 i need to get in range of 0 to 1
		m_listFaces[i].uv.y = (m_listFaces[i].normal.z * -1) / 2 + 0.5f; //here inverted Z is required just so texture is applied the way I need
		//I only want top of player to be covered with texture and the rest of player to have same color as texture bounds
		if (m_listFaces[i].normal.y < 0)
		{
			//when Y property of normal is less than 0 ( means vertex is located in the bootom part of player) I set UV bigger than 1 and clamp address mode will do the job
			m_listFaces[i].uv.x = 2;
			m_listFaces[i].uv.y = 2;
		}
	}

	//now I can create vertex buffer
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

	//most of object are using instanced drawing for a reasons explained in documentation
	//I only need one instance of this object so I create padding intane buffer with null position
	InstanceData padding;
	padding.position = Vector3(0, 0, 0);
	//create instance buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(InstanceData);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &padding;
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);

	// load a texture 
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"GreenArrow.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load player texture");
	}

	//initialisg sparks & explosion particle systems( they always follows player, but only emit particles when player collides)
	InitialiseParticleSystems();
	//initialise some player properties
	m_health = 100; //set health to 100 which is max
	m_scale = 0.6f; //set scale to 0.6

	return;
}
//function that initialises sparks particle system
void Player::InitialiseParticleSystems()
{
	m_sparks = new Sparks(m_engine);  //create sparks particle system
	m_engine->AddParticleObject(m_sparks); //add it to renderable object list
	m_sparks->SetPosition(m_position); //set position to player's position
	m_isSparksActive = false;	//set inactive
	//now explosion
	m_explosion = new Explosion(m_engine); //create explosion particle system
	m_engine->AddParticleObject(m_explosion); //add to renderable list
	m_explosion->SetPosition(m_position); //set position to player's position
	m_explosion->SetActive(false);	//set inactive

	return;
}

//render is called every frame for all objects in renderable list
void Player::Render(DirectXWrapper *wrapper, int pass)
{
	//set transformation matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale,m_scale,m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX rotation = cam->GetRotationY(); //players rotates with camera to face camera's direction
	XMMATRIX worldTransform = scaling * rotation* translation;
	m_engine->SetWorldTransform(worldTransform); //set world matrix

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

	//depending on PASS get the correct setting set
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
	wrapper->GetContext()->DrawInstanced(m_listFaces.size(), 1, 0, 0); //1 stand for 1 instance

	return;
}

//this function moves the player and updates all objects dependant on player
void Player::Update(float dTime, float yPos)
{
	//set player speed
	m_speed = 5.0f;
	//set helper bool to false
	bool isCollided = false;
	//some matrix calculations
	Vector3 moveVector = m_input->GetMovement(); //obtain movement vector from input class
	XMMATRIX rot = cam->GetRotationY(); //obtain camera rotation
	XMVECTOR moveXM = XMLoadFloat3(&moveVector);	//load move vector into 4D vector type to apply matrix
	XMVECTOR vec = XMVector3Transform(moveXM, rot); //rotate 4D move Vector
	Vector3 vec3;
	XMStoreFloat3(&vec3, vec); //store rotated move vector into vector3 type for calculations

	//First need to check for collisions to calculate position on X  & Y axis where collision are possible
	//variables for collision testing
	float newPosX = m_position.x + vec3.x * dTime * m_speed;
	float newPosZ = m_position.z + vec3.z * dTime * m_speed;
	float oldPosX = m_position.x;
	float oldPosZ = m_position.z;

	//check for wall! collisions using engine function.
	if (m_engine->CheckWallCollision(newPosX, oldPosZ, m_scale)) // First check if we colided on X axis only. 
	{
		//player has collided
		isCollided = true;
		//now if we collided on X axis, then we set X coordinate to the old one, and Z check will follow
		m_position.x = oldPosX;
	}
	else
	{
		//didn't collide on X axis - move as normal
		m_position.x = newPosX;
	}

	if (m_engine->CheckWallCollision(oldPosX, newPosZ, m_scale)) // Now check if we colided on Z axis only.
	{
		//player has collided
		isCollided = true;
		//now if we collided on Z axis, then we set Z coordinate to the old one
		m_position.z = oldPosZ;
		
	}
	else
	{
		//didn't collide on Z, set to new position
		m_position.z = newPosZ;
	}

	//now Y is precalculated and passed here as an argment so I only need to assign it
	m_position.y = yPos;

	//position calculation is finished now I can move on
	//let engine know of player position
	m_engine->UpdatePlayerPosition(m_position);
	
	//adjust light
	m_engine->MoveLight(m_position); //adjusting ight position to player position

	//setting camera here
	m_cameraMode = m_input->GetCameraMode(); // get camera mode from input class
	//check zoom options
	int zoom = m_input->GetZoom(); //get zoom level, if returns negative integer - zoom in, positive - zoom out
	if (zoom < 0)
		cam->ZoomIn(); //call zoom in on camera oject
	else if (zoom > 0)
		cam->ZoomOut(); //call zoom out
	//now can set camera in the correct mode
	if (m_cameraMode == 1) // 1 stands for first person camera
		cam->SetFirstPerson(m_position.x, m_position.y, m_position.z);
	else //else it's a 3rd person camera
		cam->SetThirdPerson(m_position.x, m_position.y, m_position.z);
	
	
	//updating sparks
	/*
		NOTE: Spark calculation are quite complicated as I need to obtain three vectors for correct setup ( vectors described in .h)
	*/
	//I only wnt want to spawn sparks when player is moving along side of the wall,
	//when player simply runs into wall on one axis and isn't moving I dont want sparks to be spawned because it won't look right
	float difX = fabs(m_position.x - oldPosX); //how much player moved on X 
	float difZ = fabs(m_position.z - oldPosZ); //how much moved on Z
	bool isMoving = true;
	if (difX == 0 && difZ == 0) //if player has collided but didn't move I deactivate sparks
	{
		m_isSparksActive = false;
		isMoving = false;
	}
	else if (isCollided && difX > 0.0f) //so if player collided & still moving on x
	{
		isMoving = true;
		//first need to calculate which way player is moving on X 
		float dif = m_position.x - oldPosX;
		if (dif > 0) //player moved +X
		{
			m_sparkDirection = Vector3(-1, 0, 0); // so behind player will be - X (Spark should travel to the opposite direction to players movement)
		}
		else
		{
			m_sparkDirection = Vector3(1, 0, 0);  //Sparks should travel to the opposite direction to players movement
		}
		//now the spark offset need to be calculated
		//if player moves on X after collision means he collided on Z axis, just need to figure out which side
		float f = m_position.z - newPosZ; // newPosZ is where player was supposed to move, therefore it is the direction to the wall which I need for offset
		if (f < 0)
		{
			m_sparkOffset = Vector3(0, 0, 1) * m_scale;
			m_sparkCross = Vector3(0, 0, -1); //cross direction is the opposite to the offset ( direction from the wall to the player)
		}
		else
		{
			//else means collision is in direction -Z
			m_sparkOffset = Vector3(0, 0, -1) * m_scale; //direction from the player to the wall * by scale, to get position on the side of player object
			m_sparkCross = Vector3(0, 0, 1); //cross direction is the opposite to the offset ( direction from the wall to the player)
		}
	}
	else if (isCollided && difZ > 0.0f) //if has collided, and movement on Z axis is not 0, means collision has happened on X axis
	{
		isMoving = true;
		// first need to calculate which way player is moving on Z
		float dif = m_position.z - oldPosZ;
		if (dif > 0) //player moved +Z
		{
			m_sparkDirection = Vector3(0, 0, -1); // so behind player will be - Z ( spark span into opposite direction to players movement)
		}
		else //player moved -Z 
		{
			m_sparkDirection = Vector3(0, 0, 1);
		}
		//now spark offset and cross
		float f = m_position.x - newPosX; //again I have to figure out which side of player collision occured
		if (f < 0)
		{
			//f < 0 means collision happened in +X direction
			m_sparkOffset = Vector3(1, 0, 0) * m_scale;
			m_sparkCross = Vector3(-1, 0, 0); //cross is opposite to offset
		}
		else
		{
			//otherwise it's -X direction where collision occured
			m_sparkOffset = Vector3(-1, 0, 0) * m_scale; //multiply by scale to get right on the side of player object
			m_sparkCross = Vector3(1, 0, 0); //cross is opposite to offset ( the direction from the wall to the object)
		}
	}
	//now vectors are precalculated
	//updating sparks
	Vector3 sparkPos = m_position + m_sparkOffset; //calculate spark position usng offset
	m_sparks->SetPosition(sparkPos ); //set sparks position
	bool spawnSparks; //helper boolean
	if (isMoving && isCollided) //i only want to spawn sparks if player has collided and also is moving
		spawnSparks = true;
	else
		spawnSparks = false;
	//if camera is in first person mode sparks don't spawn - reasons for that explained in documentation

	//also particles dont spawn if player is on ramp
	int location = m_engine->GetPlayerLocation(); //obtain player location
	if (m_cameraMode == 1 || location == 0)
		spawnSparks = false;

	//now I want to deduct health each 1/4 seconds when sparks are spawning
	if (spawnSparks)
	{
		m_healthTimer += dTime; //increment timer
		if (m_healthTimer > 0.25f) //if we spent in collision more than quarter second deduct health & reset timer
		{
			m_health -= 2; //2 health per half sec seems good ( I had tested)
			m_healthTimer = 0; 
		}
	}
	else m_healthTimer = 0; //if sparks don't spawn for a moment reset the timer
	//finally update sparks   NOTE: if spawn sparks is set to false, system wont spawn sparks but update the already active once so they finish their cycle
	m_sparks->Update(dTime,m_sparkDirection, m_sparkCross, spawnSparks);
	
	//check for collision with enemies
	bool result = m_engine->CheckEnemyCollision(m_position.x, m_position.z, m_scale); //using special engine function
	if (result)
		m_health = 0; //if collides with enemy or bullet player dies
	
	//now check for collision with pickUps
	bool pickUp = m_engine->CheckPickUpCollision(m_position.x, m_position.z, m_scale); //returns true if collided
	if (pickUp) 
	{
		m_health += 5; //if collided with pick up add 5 health
		if (m_health > 100) 
			m_health = 100; //cap the health at 100
	}

	return;
}

//dead update is special function that updates player dependant objects when player gets deactivated
void Player::DeadUpdate(float dTime)
{
	UpdateExplosion(dTime); //updating explosion , will only work if explosion is activated
	//still updating particles, but because isMoving is to false - new sparks won't spawn
	m_sparks->Update(dTime, m_sparkDirection, m_sparkCross, false); 
	//and still updating camera , camera won't move as it follows players movements, however you can still look around 
	//NOTE: not receiving camera mode from input class therefore you cant change camera mode when player is deactivated
	if (m_cameraMode == 1) // 1 stands for first person camera
		cam->SetFirstPerson(m_position.x, m_position.y, m_position.z);
	else
		cam->SetThirdPerson(m_position.x, m_position.y, m_position.z);

	return;
}

void Player::Explode() //function activates explosion paticle system
{
	m_explosion->SetActive(true);
	m_explosion->SetPosition(m_position); //set explosion position toplayer position
	return;
}

void Player::UpdateExplosion(float dTime) //update explosion system
{
	m_explosion->Update(dTime);
	return;
}




