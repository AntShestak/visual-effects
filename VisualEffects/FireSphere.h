#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"
#include "DirectXWrapper.h"

//structure for particle instance 
struct SphereFireParticleInstanceData
{
	Vector3 position;
	Vector3 color;
};
//structure for particle properties
struct SphereFireParticle
{
	Vector3 position; //particles position ( relative to its base)
	Vector3 color; //particle color
	Vector3 spawnDir;	//direction assgined to particle at spawn
	bool isActive;	
	bool isSmoke;	//is particle a smoke particle
	float lifeSpan;	//particle lifetime
	float timeAlive;	//how long particle has been alive

	//Overloading equals operator
	SphereFireParticle& operator= (const SphereFireParticle& p)
	{
		position = p.position;
		color = p.color;
		spawnDir = p.spawnDir;
		isActive = p.isActive;
		isSmoke = p.isSmoke;
		lifeSpan = p.lifeSpan;
		timeAlive = p.timeAlive;
		return *this;
	}
};
//class that renders a fire around a sphere of certain radius
class FireSphere : public IRenderable //derives from IRenderable interface
{
public:
	//constructor & destructor
	FireSphere(GraphicsEngine* ge);
	~FireSphere();
	//IRenderable interface functions
	void Initialise(DirectXWrapper* dx);
	void Render(DirectXWrapper *wrapper, int pass); // called every fram
	Vector3 GetPosition() { return m_position; }

	void UpdateInstanceBuffer(); //updates instance buffer
	void Update(float dTime, Vector3 direction); //updates particles
	
	//setters
	void SetPosition(Vector3 position);
	void SetRadius(float r);
	void SetProperties(float radius, float scale, int maxParticles, int spawnPerFrame, float lifeMultiplier, float parentSpeed); //sets some of particle system properties

private:
	//references
	GraphicsEngine* m_engine = nullptr;
	DirectXWrapper* m_directX = nullptr;
	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;
	
	//particle control functions
	void Emit(float dTime); //emits particle
	bool Kill(int index); //returns true if particle was killed
	void Move(Vector3 direction, float dTime, float value, int index); //func that applies movement to a particle
	void Color(float value, int index); //function that updates particle color
	void Deactivate(int index); //function that deactivates indexed particle
	void SortByZ(); //buble sort function
	Vector3 GetRandomVector(); //returns random vector
	Vector3 GetSmokePosition();	//return position for smoke to spawn
	//arrays
	SphereFireParticle m_particles[3000]; //particle array
	SphereFireParticleInstanceData m_instances[3000]; //instances array
	
	//particle system variables
	//main properties
	Vector3 m_position; //base osition
	float m_scale;	//scale of particle object
	//control properties
	Vector3 m_moveDirection; //direction for particle movement
	int m_currentParticleCount; //current number of active particles
	int m_maxParticles; //max number of particles possible
	int m_spawnPerFrame; 
	int m_smokeCounter;	//counts when to spawn smoke
	float m_radius; //sphere radius ( fire spawns around a sphere of suppositive radius )
	float m_defLifeSpan; //default life span of particles
	float m_speed, m_smokeSpeed, m_parentSpeed; //speed for particles and for smoke repectively, and parent object speed
	float m_timeElapsed; //time elapsed since system start
	float m_spawnTime; //time between spawns
	bool m_emitSmoke;	//if it's time to emit smoke
	
};