#include "stdafx.h"
#include "FireSphere.h"
//include texture loader
#include "DirectXTK/Inc/WICTextureLoader.h"


FireSphere::FireSphere(GraphicsEngine* ge)
{
	m_engine = ge; //reference the engine
}

FireSphere::~FireSphere()
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
	//null references
	m_engine = nullptr; 
	m_directX = nullptr;
}

#define NUM_VERTICES 6 //define number of vertices 2 triangles to make a plane = 6 vertices
// This function is called once when object is added to wrapper
void FireSphere::Initialise(DirectXWrapper *wrapper)
{
	//reference directX
	m_directX = wrapper;
	//set main object variables to defaults
	m_position = Vector3(0.0f, 0.0f, 0.0f);
	m_scale = 1.0f;
	//set particle variables
	m_moveDirection = Vector3(0, 0, 0); //set to default
	m_currentParticleCount = 0; //currently no particles active
	m_smokeCounter = 0; 
	m_speed = 0.4f;
	m_smokeSpeed = m_speed / 2.0f; //smoke should be slower
	m_timeElapsed = 0;
	m_spawnTime = 0.01f; //spawn particles every 0.01 of a second
	m_emitSmoke = false;
	//some other properties are set trought public function from parent object

	//filing particle & instance arrays with inactive particles
	for (int i = 0; i < m_maxParticles; i++)
	{
		Deactivate(i); //set particle properties to inactive properties
		//copy this to instances array
		m_instances[i].position = m_particles[i].position;
		m_instances[i].color = m_particles[i].color;

	}
	
	Vertex vertices[NUM_VERTICES]; //initialise array of vertices

	/*
	Here I will create one particle object and then using instanced rendering i will spawn many particles.
	Particle instance has two properties : position & color
	I use dynamic instance buffer to update this details each frame
	*/

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
	bd.Usage = D3D11_USAGE_DYNAMIC; //set to dynamic
	bd.ByteWidth = sizeof(m_instances);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //cpu access
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_instances[0];
	hr = wrapper->GetDevice()->CreateBuffer(&bd, &InitData, &m_instanceBuffer);
	// pVertexBuffer now points at an interface to a DirectX buffer object containing a copy of our vertices
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create particle instance buffer \n");
		return;
	};

	//creating texture resource
	hr = CreateWICTextureFromFile(wrapper->GetDevice(), L"Particle.png", &m_texture, &m_textureView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to load particle texture \n");
	}

	return;
}

// This function is called once per frame 
void FireSphere::Render(DirectXWrapper *wrapper, int pass)
{
	//set transormation matrices
	XMMATRIX scaling = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX rotation = m_engine->GetCameraRotationY(); //make particles face camera
	XMMATRIX worldTransform = scaling * rotation * translation;
	m_engine->SetWorldTransform(worldTransform); //update world transform

	//set buffers
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* buffers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(SphereFireParticleInstanceData);

	offsets[0] = 0;
	offsets[1] = 0;

	buffers[0] = m_vertexBuffer;
	buffers[1] = m_instanceBuffer;

	wrapper->GetContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	//use particle rendering settings
	m_engine->RenderParticles(m_textureView);

	// finally issue the draw call 
	wrapper->GetContext()->DrawInstanced(NUM_VERTICES, m_maxParticles, 0, 0);

	return;
}

//update dynamic buffer
void FireSphere::UpdateInstanceBuffer()
{
	//initialise subresource & clear memory
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//map instance buffer
	m_directX->GetContext()->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//load new values
	memcpy(mappedResource.pData, &m_instances, sizeof(m_instances));
	//unmap buffer
	m_directX->GetContext()->Unmap(m_instanceBuffer, 0);

	return;
}

//updating particles
void FireSphere::Update(float dTime, Vector3 moveDir)
{
	//update the direction first
	m_moveDirection = moveDir; //this is direction in which parent object is moving
	//first I am setting up helper variables that I will use for caculations
	int killedThisFrame = 0; //this variable will keep track of how many particles were killed this frame
	
	//I need to calculate vector that will create an effect that fire lasts behind the moving object (similar to fire behavior on the wind)
	//to do this I have a vector of movement, which I need to transform by inverted camera rotation on y axis only
	//I want to do this ONLY ONCE per frame, as camera rotation wont change more often
	XMMATRIX rot = m_engine->GetCameraRotationY(); //this is the same matrix I use to rotate particles
	rot = XMMatrixInverse(nullptr, rot); //inverting this matrix
	XMVECTOR vec = XMLoadFloat3(&m_moveDirection); //get moveDirection (Vector3) stored in XMVECTOR to apply matrix
	vec = XMVector3Transform(vec, rot); //transform direction of movment by inverted matrix
	Vector3 vec3; //this Vector3 will be passed to a move func
	XMStoreFloat3(&vec3, vec); //storing transformed vector in Vector3
	
	//NOW particle controls
	//first particles are emitted if it's needed
	Emit(dTime);
	
	//now looping through particles that are active at the moment as only they need updating
	for (int i = 0; i < m_currentParticleCount; i++)
	{
		//first updating particle time alive
		m_particles[i].timeAlive += dTime;
		//now killing particles if needed
		bool temp = Kill(i);
		if (temp) //if kill returns true
			killedThisFrame++; //adding to killed ths frame counter
		//this varible describes what part of it's lifetime has particle lived through
		float value = m_particles[i].timeAlive / m_particles[i].lifeSpan; //will be used in Move & Color funcs
		//now I need to update position of this particle
		Move(vec3, dTime, value, i); // passing the Vector and index of particle
		//next updating color
		Color(value, i);
	}
	
	//now sorting in z order before passing further
	SortByZ();
	//the amount of particles kille during this frame is store in variable killedThisFrame so I deduct t from current particle count
	m_currentParticleCount -= killedThisFrame; // I couldn't do this in a loop that is based on current particle variable same Sort function
	
	//now updating instances array
	for (int i = 0; i < m_maxParticles; i++)
	{
		m_instances[i].position = m_particles[i].position;
		m_instances[i].color = m_particles[i].color;
	}
	//and updating instance buffer
	UpdateInstanceBuffer();
	
}

void FireSphere::Emit(float dTime)
{
	m_timeElapsed += dTime; //updating time since last spawn
	
	//if it's time to spawn new particles && there's space for new particles
	if (m_timeElapsed >= m_spawnTime && m_currentParticleCount < m_maxParticles - m_spawnPerFrame)
	{
		//reset timeElapsed
		m_timeElapsed = 0.0f;

		for (int i = 0; i < m_spawnPerFrame; i++) // spawning the number of particles
		{
			m_currentParticleCount++; //adding to current particle count
			m_smokeCounter++; // smoke is spawned in a slightly different way, so I need this counter

			if (m_smokeCounter == 3) //every 3rd particle is smoke particle
			{
				m_emitSmoke = true; //using this to emit smoke particle with different settings that fire has
				m_smokeCounter = 0; //nulling smoke counter
			}
			else m_emitSmoke = false; //if counter not reached I'm spawning fire partile

			Vector3 newPosition; //variable to hold position for particle to spawn
			if (!m_emitSmoke)
				newPosition = GetRandomVector(); //this function gets position on sphere surface of certain radius, used to spawn fire particles
			else
				newPosition = GetSmokePosition(); // this function gets a position in are on top of phere, thats where I want to spawn my smoke particles

			int index = m_currentParticleCount - 1; //index for the new particle in the list, -1 because I have already done ++ for this particle above
			//some properties of particle are the same between smoke && fire
			m_particles[index].position = newPosition; //position to spawn
			m_particles[index].spawnDir = newPosition;	//direction is used for fire movement calculations, and can be ignored for smoke
			m_particles[index].isActive = true; //particle is now active particle
			m_particles[index].timeAlive = 0.0f; //time alive is yet 0
			if (!m_emitSmoke) //if spawning fire particle
			{
				m_particles[index].isSmoke = false; //it's not smoke
				int ranvar = rand() % 100  + 100;	//random integer in range of 100 to 200
				float r = (float)ranvar / 100.0f; // turning it to float in range of 1.0 to 2.0
				m_particles[index].lifeSpan = m_defLifeSpan * r; // so every particle has a bit of a random lifetime based on default lifetime
			}
			else //else we are spawning smoke
			{
				m_particles[index].isSmoke = true; // it's smoke
				int ranvar = rand() % 100 + 100;	//same randomisation as with fire lifetime
				float r = (float)ranvar / 100.0f;
				m_particles[index].lifeSpan = m_defLifeSpan * r * 1.5f; // smoke have slightly longer life than fire
			}
			//color is set to 0 for inactive particles so I leave like it is for now
		}
	}

	return;
}

bool FireSphere::Kill(int index) //checks if paticle needs deactivating
{
	if (m_particles[index].timeAlive > m_particles[index].lifeSpan) //if particle lived too long
	{
		Deactivate(index); //deactivate that particle
		return true; //if particle was killed returns true
	}
	return false; // otherwise returns false
}

void FireSphere::Deactivate(int index) //function that sets particle inactive
{
	//setting all particle parameters 
	m_particles[index].isActive = false;
	m_particles[index].isSmoke = false; 
	// position is -100 on Z axis, so later in an update function SortByZ function will do the job of moving this deactivated particle to the end of the list
	m_particles[index].position = Vector3(0, -100.0f, -100.0f); 
	m_particles[index].color = Vector3(0, 0, 0);
	m_particles[index].spawnDir = Vector3(0, 0, 0);
	m_particles[index].timeAlive = 0.0f;
	m_particles[index].lifeSpan = 0.0f;
}

void FireSphere::Move(Vector3 moveDirection, float dTime, float value, int index)	//func that applies movement to a particle
{
	if (m_particles[index].isActive) // I want to apply movement only if particle is active (it might have been killed this frame)
	{
		Vector3 movement;	//vector to store movement
		
		//now I ant to apply a different movement depending if particle is Smoke or it is a Fire particle
		if (!m_particles[index].isSmoke) //if particle is not smoke
		{
			//closer to its birth particle moves in the direction of spawn, as longer it lives it moves more upwards
			movement = (m_particles[index].spawnDir * (1.0f - value) + Vector3(0.0f, 1.0f, 0.0f) * value * 50.0f) * 0.250;
			//now I want to add random fluctuation on X axis to create more interesting movement
			int var = rand() % 1000 - 500; //range -500 to 500
			float xRand = var / 1000.0f * 25.0f ; //now range is -0.05 to 0.05
			movement.x += xRand; //applying randomised value on X axis
			//now applying calculated movement vector to particle position
			//now I need to also apply the vector thatwas passed to this function
			movement -= moveDirection * value * m_parentSpeed * 3.0f; //as faster parent object moves as more fire will be falling behind
			movement *= m_speed * dTime;
			
		}
		else //if it is smoke
		{
			//now to add randomized x value
			int var = rand() % 1000 - 500;
			float xRand = var / 1000.0f * 40.0f; //smoke will randomly move in X axis evey frame by the amount in range of -0.05 to 0.05
			movement.x += xRand;
			//randomised Y value
			var = rand() % 500 + 500;
			float yRand = (float)var / 1000.0f; //on Y axis range will from 0.5 to 1.0
			movement.y +=  yRand * 15.0f;
			//now applying calculated movement vector to particle position
			//now I need to also apply the vector thatwas passed to this function
			movement -= moveDirection * value * m_parentSpeed * 3.0f; //as faster parent object moves as more fire will be falling behind
			movement *= m_smokeSpeed * dTime;
		}
		
		m_particles[index].position += movement; //apply movement
	}
}
void FireSphere::Color(float value, int index)	//func that updates color of the particle, value is described above in update function
{
	Vector3 col; //vector to store color
	//first check if it's a fire or smoke particle
	if (!m_particles[index].isSmoke) //if fire
	{
		if (value > 1) //if value is more than 1 for some reason
			value = 1; //set it to 1
		//color will transform from yellow(1,1,0) to red(1,0,0) during it's lifetime ( linear interpolation )
		col = Vector3(1.0f, (1 - value), 0.0f);
	}
	else //if smoke
	{
		//I need a random shade of gray
		int var = rand() % 150 + 60; //range 60 to 210
		float c = (float)var / 1000.0f; //range is now 0.06 to 0.21
		col = Vector3(c, c, c); //creating a shade of grey 
	}
	//now applying calculated color
	m_particles[index].color = col;

	return;
}

Vector3 FireSphere::GetRandomVector()
{
	//Trying for Gaussian distribution to spawn particles on sphere of set radius
	float x, y, z;
	int randX = rand() % 1000 - 500;
	int randY = rand() % 1000 - 500;
	int randZ = rand() % 1000 - 500;
	//manual normalisation
	x = (float)randX * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2))) * m_radius;
	y = (float)randY * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2))) * m_radius;
	z = (float)randZ * (1 / sqrt(pow(randX, 2) + pow(randY, 2) + pow(randZ, 2))) * m_radius;
	//adjust to scaling
	x = x  / m_scale;
	y = y  / m_scale;
	z = z / m_scale;
	
	return Vector3(x, y, z);
}

Vector3 FireSphere::GetSmokePosition()
{
	//smoke spawns around some part near top of the sphere
	float x, y, z;

	float var = rand() % 1400 - 700; 
	x = (float)var / 1000.0f * m_radius / m_scale; //random value from 0 to 0.7
	
	var = rand() % 1400 - 700;
	z = (float)var / 1000.0f * m_radius / m_scale; //random value from 0 to 0.7

	var = rand() % 200 + 600; 
	y = (float)var / 1000.0f * m_radius / m_scale; // random value from  0.6 to 0.8

	return Vector3(x, y, z);
}

void FireSphere::SetPosition(Vector3 position)
{
	m_position = position;
}

void FireSphere::SortByZ()
{
	//bubble sort that sorts array in z order
	SphereFireParticle temp;//temporary storage
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

//sets some of particles properties from it's parent class
void FireSphere::SetProperties(float radius, float scale, int maxParticles, int spawnPerFrame, float defLifeSpan, float parentSpeed)
{
	m_radius = radius; //spawn radius
	m_scale = scale;	//particle scale
	m_maxParticles = maxParticles; //max nuber of particles
	m_spawnPerFrame = spawnPerFrame; //how many to spawn per frame
	m_defLifeSpan = defLifeSpan; //default life span
	m_parentSpeed = parentSpeed; //parent object speed

	return;
}

