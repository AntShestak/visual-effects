#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"

//structure for particle instances
struct ExplosionParticleInstanceData
{
	Vector3 position; 
	Vector3 color;
};

//structure with properties for every particle
struct ExplosionParticle
{
	Vector3 position;
	Vector3 color;
	Vector3 spawnDir; //diection assigned when particle spawns
	bool isActive; 
	float lifeSpan; //particle lifetime
	float timeAlive;	//time particle spent alive

	//Overloading equals operator
	ExplosionParticle& operator= (const ExplosionParticle& p)
	{
		position = p.position;
		color = p.color;
		spawnDir = p.spawnDir;
		isActive = p.isActive;
		lifeSpan = p.lifeSpan;
		timeAlive = p.timeAlive;
		return *this;
	}
};


class Explosion : public IRenderable
{
public:
	//constructor & destructor
	Explosion(GraphicsEngine* ge);
	~Explosion();
	//IRenderable functions
	void Initialise(DirectXWrapper* dx);  //initialise is called once when object is added to renderable list
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }
	
	void Update(float dTime); //updates particle system
	void UpdateInstanceBuffer(); //updates dynamic instance buffer
	
	//setters
	void SetPosition(Vector3 position);
	void SetActive(bool isActive) { m_isActive = isActive; } 

	

private:
	//particle behaviour functions
	void Emit(); //emits particles
	bool Kill(int index); //returns true if particle with index passd is killed
	void Move(int index, float dtime); //applies movement to the particle indexed
	void UpdateColor(int index); //updates indexed particle color
	void DeactivateParticle(int index); //deactivates indexed particle
	void SortByZ(); //sorts particles in Z order
	Vector3 GetRandomVector(); //returns random vector
	
	//reference to engine & directX
	GraphicsEngine*				m_engine = nullptr;
	DirectXWrapper*				m_directX = nullptr;
	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource*				m_texture = nullptr;
	ID3D11ShaderResourceView*	m_textureView = nullptr;

	//particle arrays
	ExplosionParticle m_particles[1000]; //this one is used for calculation
	ExplosionParticleInstanceData m_instances[1000];	//this used for rendering

	//particle system variables
	//main properties
	Vector3 m_position; 
	float m_scale; //scale of particle object
	//control properties
	int m_maxParticles; //max possible particles
	int m_currentParticleCount; //number of particles currently active
	int m_waveCounter; //emit particles in waves to less load the processor, this variable counts waves
	int m_maxWaves; //max waves count
	float m_speed; //particle movement speed
	float m_timeElapsed; //time elapsed since system was activated
	float m_defLifeSpan; //default lifespan of particle
	bool m_isActive;	//is system active
	
};