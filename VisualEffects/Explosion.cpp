#include "stdafx.h"
#include "Explosion.h"
#include "DirectXWrapper.h"
//texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"

//class that renders explosion
Explosion::Explosion(GraphicsEngine* ge)
{
	m_engine = ge; //engine reference
}

Explosion::~Explosion()
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
	//null reference pointers
	m_engine = nullptr;
	m_directX = nullptr;
}

#define NUM_VERTICES 6 //define number of vertices 2 triangles to make a plane = 6 vertices
// This function is called once when object is added to wrapper
void Explosion::Initialise(DirectXWrapper *wrapper)
{
	m_directX = wrapper; //directX reference used to update instance buffer
	//temporary set position & scale to default values
	m_position = Vector3(0, 0, 0);
	m_scale = 1.0f;

	//now setting all other variables
	m_maxParticles = ARRAYSIZE(m_particles); //max particles is size of array defined in .h
	m_currentParticleCount = 0; //no particles currently active
	m_waveCounter = 0; 
	m_maxWaves = 5;	//spawn particles in 5 waves 
	m_speed = 5.0f; 
	m_timeElapsed = 0;
	m_defLifeSpan = 1.0f; //default particle lifespan
	m_isActive = false; //system is not crrently active
	
	//filling both arrays with inactive particles
	for (int i = 0; i < m_maxParticles; i++)
	{
		//deactivate function sets index particle to it's incative properties
		DeactivateParticle(i);
		//filling instances array
		m_instances[i].position = m_particles[i].position;
		m_instances[i].color = m_particles[i].color;
	}

	/*
		Here I will create one particle object and then using instanced rendering i will spawn many particles.
		Particle instance has two properties : position & color
		I use dynamic instance buffer to update this details each frame
	*/
	
	Vertex vertices[NUM_VERTICES]; //vertex array

	//plane consists of 2 triangles 
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
	bd.Usage = D3D11_USAGE_DYNAMIC; //usage is set to dynamic
	bd.ByteWidth = sizeof(ExplosionParticleInstanceData)* m_maxParticles;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //CPU access is set
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_instances[0];
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create particle instance buffer \n");
		return;
	};

	//load the texture for particle
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Particle.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load particle texture \n");
	}

	return;
}

// This function is called once per frame 
void Explosion::Render(DirectXWrapper *wrapper, int pass)
{
	//set the world transform which will be the base for particle calculations 
	XMMATRIX scaling = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX rotation = m_engine->GetCameraRotationY();
	XMMATRIX worldTransform = scaling * rotation * translation;
	m_engine->SetWorldTransform(worldTransform); //update world transform

	// set buffers
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(ExplosionParticleInstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = m_vertexBuffer;
	buffers[1] = m_instanceBuffer;

	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	//call render for correct setup
	m_engine->RenderParticles(m_textureView);

	// finally issue the draw call 
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, m_maxParticles, 0, 0);

}

void Explosion::Update(float dTime) //called each frame
{	
	//particles get updaed only if system is active
	if (m_isActive)
	{
		m_timeElapsed += dTime; //keep track of time elapsed
		//variable that helps to keep track of particle count
		int killsThisFrame = 0; //nothing yet is killed this frame
		//emitting in number of waves
		if (m_waveCounter < m_maxWaves) //if waves has reached max waves stop emitting particles
		{
			m_waveCounter++;	//increase counter
			Emit();		//emit particles
		}
		//now looping through the current particles active and updating them (NOTE: not looping through the max particles , it's more expensive)
		for (int i = 0; i < m_currentParticleCount; i++)
		{
			//updating current particle

			//update it's timeAlive
			m_particles[i].timeAlive += dTime;
			//now check if it needs to be killed
			bool result = Kill(i);
			if (result)
			{
				//if needs to be killed increase the counter
				killsThisFrame++;
			}
			else //if still alive
			{
				//now moving the particle
				Move(i, dTime);
				//calculate correct color
				UpdateColor(i);
			}
		}
		
		//before updating array have to sort it
		SortByZ();
		//now I adjust current particle count, couldn't before because loop & sort are dependant on this number
		m_currentParticleCount -= killsThisFrame;
		
		//now updating instances array
		for (int i = 0; i < m_maxParticles; i++)
		{
			m_instances[i].position = m_particles[i].position;
			m_instances[i].color = m_particles[i].color;
		}
		
		//and updating instance buffer
		UpdateInstanceBuffer();
	}

	return;
}

void Explosion::UpdateInstanceBuffer() //update dynamic buffer
{
	//initialise subresource & clear memory
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//map instance buffer
	m_directX->GetContext()->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//load new values
	memcpy(mappedResource.pData, m_instances, sizeof(m_instances));
	//unmap buffer
	m_directX->GetContext()->Unmap(m_instanceBuffer, 0);

}

bool Explosion::Kill(int index) //function checks if particle needs to be killed
{
	if (m_particles[index].lifeSpan < m_particles[index].timeAlive) //if particle lived longer than it's lifetime is
	{
		DeactivateParticle(index); //deativate that particle
		return true; //and return true
	}

	return false; //if it's still can live return false
}

void Explosion::Emit() //function emits particle wave
{
	int spawnPerFrame = m_maxParticles / m_maxWaves; //calculate how many particles to spawn
	for (int i = 0; i < spawnPerFrame; i++)
	{
		m_currentParticleCount++; //increment particle count
		int index = m_currentParticleCount - 1; //as I already incrementd particle count , current particle will be -1 (because array starts at 0)
		//spawn particle here
		Vector3 pos = Vector3(0, 0, 0); //set to base position at spawn time
		Vector3 dir = GetRandomVector(); //get random vector for further movement
		//assign this values
		m_particles[index].position = pos;
		m_particles[index].spawnDir = dir;

		m_particles[index].isActive = true; //set active to true
		m_particles[index].timeAlive = 0; //it has just spawned so time alive is 0
		//slightly randdomising particle lifetime
		int var = rand() % 40 + 80; //random integer in range 80 to 120
		float r = (float)var / 100.0f; //get decimal value
		m_particles[index].lifeSpan = m_defLifeSpan * r; //each article has slightly different lifespan based on default value
		//the rest of particle properties are left as they are on deactivated particle at the time being to be updating after emition
	}
	return;
}

void Explosion::Move(int index, float dTime) //function that moves particles
{
	//i want particles to move slower as their life comes to an end
	//calculate percentage of lifetime (in decimals)
	float value = m_particles[index].timeAlive / m_particles[index].lifeSpan;
	//movement vector
	//move in direction (assigned at spawn) by speed multiplied by 0.8 - value explained below
	//0.8 - value: makes particles move slower as longer they live, but by the end ( the last 0.2 of their life) they even start moving in opposite direction
	Vector3 movement = m_particles[index].spawnDir * dTime * m_speed * (0.8f - value);
	//add movement
	m_particles[index].position += movement;

	return;
}
 
void Explosion::UpdateColor(int index)
{
	//calculate percentage of lifetime (in decimals)
	float value = m_particles[index].timeAlive / m_particles[index].lifeSpan;
	//white will be default color
	Vector3 col = Vector3(1.0f, 1.0f, 1.0f);

	//now I want particles to start with yellow, and get to red in the first quarter of their existence
	if (value < 0.25)
	{
		value *= 4; //multiply by 4 as I need full cycle in quarter of particle lifetime
		//creating translation from yellow to red
		col = Vector3(1.0f, value, 0.0f); //in the start G = aroud 0, by the end G = 1 ( G for green , RGB)
	}
	else if (value < 0.5f) //in the next quarter of lifetime color transforms from red to white
	{
		value -= 0.25; //because I calculate form period from 0.25 to 0.5
		value *= 4; //I need full cycle in quarter of lifetime
		col = Vector3(1.0f, 1.0f, value); //in the start Blue value will be around 0 , by the end around 1
	}
	//assign calculated color
	m_particles[index].color = col;
		
	return;
}

void Explosion::DeactivateParticle(int index)
{
	//set indexed particle properties
	m_particles[index].isActive = false;
	m_particles[index].position = Vector3(0, -100.0f, -100.0f); //-100 on Z is important, this makes SortByZ move deactivated particles to the end (killing two birds...)
	m_particles[index].spawnDir = Vector3(0, 0, 0);
	m_particles[index].timeAlive = 0.0f;
	m_particles[index].lifeSpan = 0.0f;
	m_particles[index].color = Vector3(0, 0, 0);
	
	return;
}

Vector3 Explosion::GetRandomVector()
{
	//Trial for Gaussian distribution technique so the return value lays on sphere of certain radius 
	
	float x, y, z;
	//random value  from -0.5 to 0.5
	int randX = rand() % 1000 - 500;
	int randY = rand() % 1000 - 500;
	int randZ = rand() % 1000 - 500;
	//manually normalise ( formula obtained from internet where Gaussian Distribution was explained)
	x = (float)randX * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2)));
	y = (float)randY * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2)));
	z = (float)randZ * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2)));
	
	x = x / m_scale;
	y = y / m_scale;
	z = z / m_scale;
	//need to muptilply by radius but it's 1
	return Vector3(x, y, z);

}

void Explosion::SetPosition(Vector3 position)
{
	m_position = position;
}

//function sorts particle in Z order so they get rendered properly and don't cover each other
void Explosion::SortByZ()
{
	//simple bubble sort
	ExplosionParticle temp; //temporary storage
	for (int i = 0; i < m_currentParticleCount; i++) //iterate through particles
	{
		for (int j = m_currentParticleCount - 1; j > 0; j--) //once more (bt this time from the back of the list
		{
			if (m_particles[j].position.z > m_particles[j - 1].position.z) //if Z is higher tha one before swap them
			{
				temp = m_particles[j - 1]; //temporary store
				m_particles[j - 1] = m_particles[j]; //swap
				m_particles[j] = temp; //return store particle
			}
		}
	}

	return;
}



