#include "stdafx.h"
#include "Sparks.h"

//include texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"

//class that renders sparks
Sparks::Sparks(GraphicsEngine* ge)
{
	m_engine = ge; //reference to graphics engine
}

Sparks::~Sparks()
{
	//release rendering resources & their pointers
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = nullptr;
	if (m_instanceBuffer) m_instanceBuffer->Release();
	m_instanceBuffer = nullptr;
	if (m_texture) m_texture->Release();
	m_texture = nullptr;
	if (m_textureView) m_textureView->Release();
	m_textureView = nullptr;
	//null references
	m_engine = nullptr;
	m_dx = nullptr;
}

#define NUM_VERTICES 6 //to create a plane I need two triangles = 6 vertices
// This function is called once at startup when the object is added to renderable list
void Sparks::Initialise(DirectXWrapper *wrapper)
{
	m_dx = wrapper; //reference directX

	//set base position to default value
	m_position = Vector3(0.0f, 1.0f, 0.0f);
	m_scale = 0.1f;
	//set control variables
	m_maxParticles = ARRAYSIZE(m_particles); //set max particles to the array size initialised in .h
	m_currentParticleCount = 0; //currently no particles active
	m_speed = 20.5f; //move speed
	m_defLifeSpan = 0.65f; 
	m_isRunning = false; //system is not running at the moment
	m_currentTime = 0.0f;
	m_defSpawnTime = 0.001f;
	m_spawnTime = 1.5f; //random value will be attered after first particle spawns

	//temporary nulling instance positions
	for (int i = 0; i < m_maxParticles; i++)
	{
		m_instances[i].position = Vector3(0, -10, 0); //set position
		m_particles[i].position = m_instances[i].position; //copy position
		m_instances[i].color = Vector3(0, 0, 0); //color set to default
		m_particles[i].color = m_instances[i].color; //copy color
		//now only particle data set it all to default values
		m_particles[i].spawnDir = Vector3(0, 0, 0);
		m_particles[i].isActive = false;
		m_particles[i].timeAlive = 0.0f;
		m_particles[i].lifeSpan = 0.0f;
	}

	/*
	Here I will create one particle object and then using instanced rendering i will spawn many particles.
	Particle instance has two properties : position & color
	I use dynamic instance buffer to update this details each frame
	*/
	

	//vertex array
	Vertex vertices[NUM_VERTICES];

	//plane consists of two triangles
	vertices[0] = { Vector3(-1, 1, 0), Vector2(0, 1), Vector3(0, 0, 0) };
	vertices[1] = { Vector3(1, 1, 0), Vector2(1, 1), Vector3(0, 0, 0) };
	vertices[2] = { Vector3(-1, -1, 0), Vector2(0, 0), Vector3(0, 0, 0) }; //triangle 1
	vertices[3] = { Vector3(-1, -1, 0), Vector2(0, 0), Vector3(0, 0, 0) };
	vertices[4] = { Vector3(1, 1, 0), Vector2(1, 1), Vector3(0, 0, 0) };
	vertices[5] = { Vector3(1, -1, 0), Vector2(1, 0), Vector3(0, 0, 0) }; //triangle 2

	// creating vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HRESULT hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);

	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create vertex buffer");
		return;
	}

	//creating dynamic instance buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;  //usage set to dynamic
	bd.ByteWidth = sizeof(SparkParticleInstanceData)* m_maxParticles;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //cpu access
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_instances[0];
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create particle instance buffer \n");
		return;
	};

	//load testure
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Particle.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load particle texture \n");
	}

	return;

}

// This function is called once per frame
void Sparks::Render(DirectXWrapper *wrapper, int pass)
{
	//set the matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX rotation = m_engine->GetCameraRotationY(); //rotates particle to face camera
	XMMATRIX worldTransform = scaling * rotation * translation;
	m_engine->SetWorldTransform(worldTransform); //update final world matrix

	//set buffers
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(SparkParticleInstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = m_vertexBuffer;
	buffers[1] = m_instanceBuffer;

	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	//call render particles for correct settings
	m_engine->RenderParticles(m_textureView);

	// finally issue the draw call 
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, m_maxParticles, 0, 0);

	return;
}

//updates particles. direction is a diretion in which object moves, cross is vector from the wall object collided to to an empty space, bool describes if object is moving
void Sparks::Update(float dTime, Vector3 direction, Vector3 cross, bool isMoving)
{
	m_currentTime += dTime; //time elapsed since last spawn

	if (m_currentTime >= m_spawnTime && isMoving) //only if player is moving Im spawning new ( also if it's time to spawn a new spark)
	{
		EmitSpark(direction, cross); //pass the vectors to emit spark
		m_currentTime = 0.0f; //reset timer
		//randomising spawn time (so they spawn after slightly different intervals
		int r = rand() % 100 + 50;
		float var = (float)r / 100.0f; //float in range of 0.5 to 1.5
		m_spawnTime = m_defSpawnTime * var;
	}
	
	if (m_currentParticleCount > 0) // only going throgh loop if there are partices to update
	{
		//update time alive
		for (int b = 0; b < m_currentParticleCount; b++)
		{
			m_particles[b].timeAlive += dTime;
		}

		//kill particles first if needed
		KillParticles(dTime);
		//move particles
		MoveParticles(dTime);

		//before updating array have to sort
		SortByZ();
		//now updating instances array
		for (int i = 0; i < m_maxParticles; i++)
		{
			m_instances[i].position = m_particles[i].position;
			m_instances[i].color = m_particles[i].color;
		}

		//and updating instance buffer
		UpdateInstanceBuffer(m_dx);
	}
	
}

void Sparks::UpdateInstanceBuffer(DirectXWrapper* wrapper) //update dynamic buffer
{
	//initialise subresource & clear memory
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//map instance buffer
	wrapper->GetContext()->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//load new values
	memcpy(mappedResource.pData, m_instances, sizeof(m_instances));
	//unmap buffer
	wrapper->GetContext()->Unmap(m_instanceBuffer, 0);

	return;

}

void Sparks::KillParticles(float dTime)
{
	for (int i = 0; i<m_maxParticles; i++) //loop through particles
	{
		if ((m_particles[i].isActive == true) && (m_particles[i].timeAlive > m_particles[i].lifeSpan)) //if is active and lived too long
		{
			DeactivateParticle(i); //deactivate particle

			// Now shift all the live particles back up the array to erase the destroyed particle and keep the array sorted correctly.
			for (int j = i; j<m_maxParticles - 1; j++)
			{
				m_particles[j] = m_particles[j + 1];
			}
		}
	}

	return;
}

void Sparks::EmitSpark(Vector3 dir, Vector3 cross)
{
	//if theres space in array for a new particle
	if (m_currentParticleCount < m_maxParticles - 1)
	{ 
			m_currentParticleCount++; //increase cunter

			Vector3 newPosition = Vector3(0, 0, 0); 

			Vector3 direction = GetRandomVector(dir, cross); //get random direction to move using vector obtaind form the parent bject

			int index = m_currentParticleCount - 1;
			// Now insert it into the particle array in the correct depth order.
			m_particles[index].position = newPosition;
			m_particles[index].spawnDir = direction; //assign precalculated direction
			m_particles[index].isActive = true;
			m_particles[index].timeAlive = 0.0f;
			//slightly randomise lifespan
			int ranvar = rand() % 50 + 50;
			float r = (float)ranvar / 100.0f; //float in range 0.5 to 1.0
			m_particles[index].lifeSpan = m_defLifeSpan * r;
			//assign bright green color to all particles
			m_particles[index].color = Vector3(0.0, 1.0, 0.0); //have tried players texture colors but bright green looks so much better for sparks
	}
	
}

void Sparks::MoveParticles(float dTime)
{
	//firt preparing the matrix 
	XMMATRIX rot = m_engine->GetCameraRotationY(); //this is the same matrix I use to rotate particles
	rot = XMMatrixInverse(nullptr, rot); //inverting this matrix
	
	// particles movement will use spawn direction f the particles and up vector as longer particle is alive, as bigger will be value of up vector
	for (int i = 0; i<m_currentParticleCount; i++)
	{
		m_direction = m_particles[i].spawnDir;
		//rotating the direction vector
		XMVECTOR vec = XMLoadFloat3(&m_direction); //get span direction (Vector3) stored in XMVECTOR to apply matrix
		vec = XMVector3Transform(vec, rot); //transform direction of movment by inverted matrix
		Vector3 vec3; //this Vector3 will be passed to a move func
		XMStoreFloat3(&vec3, vec); //storing transformed vector in Vector3

		Vector3 movement;
	
		//calculating value ( which part of it's life time has particle lived through) 
		float value = m_particles[i].timeAlive / m_particles[i].lifeSpan - 0.75f; //minus 0.75 because I don't want the end value to be 1, but 0.25 
		//precalculated direction vec3 is used here. As longer particles lives as less it moves here
		movement = vec3 * (1 - value) * dTime * m_speed;

		//now need to add gravity
		value = m_particles[i].timeAlive / m_particles[i].lifeSpan;  //which part of it's life time has particle lived through
		float gravity = value * dTime * m_speed/2; //movement lower gets stronger as movement in spawn direction decreases
		movement.y -= gravity;
		//adjust position
		m_particles[i].position += movement;

	}

	return;
}

void Sparks::DeactivateParticle(int index)
{
	//set important value to 0, other values will be rewritten
	m_particles[index].isActive = false;
	m_particles[index].position = Vector3(0, -100.0f, 0);
	m_particles[index].timeAlive = 0.0f;
	m_currentParticleCount--;
}

Vector3 Sparks::GetRandomVector(Vector3 direction, Vector3 cross)
{
	//here i had to do some calculations
	//vector should be in the direction of "DIRECTION" vector and slightly in direction of "CROSS" vector and slightly up
	
	Vector3 vector = direction;
	//get random number
	int var = rand() % 200 + 50;
	float r = (float)var / 1000.0f; //float in range 0.05 to 0.25
	vector += cross * r; //add cross multiplied by the radom number
	//nother random
	var = rand() % 500 + 350;
	r = (float)var / 1000.0f; //float in range 0.5 to 0.85
	vector += Vector3(0, 1, 0) * r; //add up vector multiplied by new random

	return vector;
}


void Sparks::SetPosition(Vector3 position) //sets base positon of particle system
{
	m_position = position;
}

void Sparks::SortByZ()
{
	//trying to implement bubble sort
	SparkParticle temp;  //temporary stora
	for (int i = 0; i < m_currentParticleCount; i++) //iterate through particles
	{
		for (int j = m_currentParticleCount - 1; j > 1; j--) //once more (bt this time from the back of the list
		{
			if (m_particles[j].position.z > m_particles[j - 1].position.z) //if Z is higher tha one before swap them
			{
				temp = m_particles[j - 1]; //temporary store
				m_particles[j - 1] = m_particles[j]; //swap
				m_particles[j] = temp; //return stored particle
			}
		}
	}

	return;
}


