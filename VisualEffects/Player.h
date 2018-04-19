#pragma once

#include "IRenderable.h"
//objects used in class
#include "GraphicsEngine.h"
#include "Camera.h"
#include "Input.h"
#include "Sparks.h"
#include "Explosion.h"

//class that renders and controls player object
class Player :public IRenderable //derives from IRederable
{
public:
	Player(GraphicsEngine *ge);
	~Player();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition() { return m_position; }
	
	//Setters
	void SetPosition(float x, float y, float z) { m_position.x = x; m_position.y = y; m_position.z = z; }
	void SetHealth(int health) { m_health = health; }
	//Getters
	float GetScale() { return m_scale; }
	int GetHealth(){ return m_health; } //returns health of the player

	//update function is responsible for movements and controls other behaviours
	void Update(float dTime, float yPos); //Y is precalculated in game.cpp and passed every frame using this function
	//this update function is used when player is dead to still be able to update some player dependant objects(look around and update particles)
	void DeadUpdate(float dTime); 
	
	void Explode(); //starts explosion particle system
	void UpdateExplosion(float dTime); //updates explosion particle system

private:
	//private functions
	//sphere generation
	void PushBackTriangle(Vertex &vert1, Vertex &vert2, Vertex &vert3); //add three vertices to the list ( 3 vertices that make one face)
	Vertex GetMidPoint(Vertex &vert1, Vertex &vert2); //returns middle point between two vertices
	void Subdivide(Vertex &vert1, Vertex &vert2, Vertex &vert3, int depth); //subdivides triangle (made from three vertices passed) into 4 triangles
	//particles sytem
	void InitialiseParticleSystems(); //initialises sparks particle system

	//game object references
	GraphicsEngine* m_engine = nullptr;
	Camera *cam = nullptr;
	Input* m_input = nullptr;
	Sparks* m_sparks = nullptr;
	Explosion* m_explosion = nullptr;

	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource*				m_texture = nullptr;
	ID3D11ShaderResourceView*	m_textureView = nullptr;

	vector<Vertex> m_listFaces; //list of vertices where every three consecutive represent a triangle

	//variables
	//main properties
	Vector3 m_rotation = Vector3(0, 0, 0);
	Vector3 m_rotationAxis = Vector3(1, 0, 0);
	Vector3 m_position;
	float m_angle;
	float m_scale;
	float m_speed;
	//health control variables
	int m_health;
	float m_healthTimer; //this is used to deduct health each time unit (second or 1/4 second) , but not each frame
	
	//camera control variables
	int m_cameraMode; //here 1 is first person , 3 is third person
	
	//spark control variables
	bool m_isSparksActive;
	Vector3 m_sparkOffset; //this variables describes distance between player's center and particle emition position
	Vector3 m_sparkDirection; //thats the direction inverse to player movement (so sparks are emitted to the back of the player)
	Vector3 m_sparkCross;	//I called this spark cross vector, it's a vector FROM the wall to the player


};