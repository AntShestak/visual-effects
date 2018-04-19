#pragma once

#include "IRenderable.h" //include Irenderable interface for all renderable objects
#include "DirectXWrapper.h"
#include "GraphicsEngine.h" //include GraphicEngine for reference

//sructure for particle instance data
struct SparkParticleInstanceData 
{
	Vector3 position;
	Vector3 color;
};

struct SparkParticle //struct for particles containing properties for each particle
{
	Vector3 position;
	Vector3 color;
	Vector3 spawnDir;	//direction assigned to particle when it's emited
	bool isActive;
	float lifeSpan;		//lifeSpan of particle
	float timeAlive;	//how long particle has been alive
	//Overloading equals operator
	SparkParticle& operator= (const SparkParticle& p)
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

//class responsible for emitting sark particles
class Sparks : public IRenderable	 //derives from IRenderable interface
{
public:
	Sparks(GraphicsEngine* ge);	//constructor
	~Sparks();					//destructor
	
	//IRenderable interface function that need to be overriden
	void Initialise(DirectXWrapper* dx);			//called when adding as a renderable object
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }		//returns current position of emittion point

	void SetPosition(Vector3 position);	//set position from which particles are emitted

	void Update(float dTime, Vector3 direction, Vector3 cross, bool isMoving);	//updates spark particles
	
private:
	//private functions
	void UpdateInstanceBuffer(DirectXWrapper* wrapper);			//updating dynamic instance buffer
	//particle update functions
	void EmitSpark(Vector3 direction, Vector3 cross); //emits a spark
	void KillParticles(float dTime); //removes dead particles
	void MoveParticles(float dTime);	//moves particles
	void DeactivateParticle(int index);	//sets indexed particle inactive
	void SortByZ();			//sorts particles in Z order
	Vector3 GetRandomVector(Vector3 direction , Vector3 cross); //gives a random vector based on direction vector

	//references
	GraphicsEngine* m_engine = nullptr;
	DirectXWrapper* m_dx = nullptr;
	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;
	

	//private variables
	//arrays of particles
	SparkParticle m_particles[100];				//this array is used for calculations
	SparkParticleInstanceData m_instances[100];	//instance array is used for rendering

	//particle system variables
	//main variables
	Vector3 m_position;
	float m_scale; //particle object scale
	//control variables
	Vector3 m_direction; //particle movement direction applied at spawn
	int m_maxParticles; //max particle count
	int m_currentParticleCount; //current active particle count
	float m_speed; //movement speed
	float m_defLifeSpan; //default particle life span
	float m_defSpawnTime; //def spawn time
	float m_spawnTime; //i don't want to sawn particles at the same rent, so this variable will be randomised based on defspawn time
	float m_currentTime; //current time of system running
	bool m_isRunning; //is system running
	
};