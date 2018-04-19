#pragma once
//include bullet interfaces
#include "IRenderable.h"
#include "ICollidable.h"

#include "GraphicsEngine.h"
#include "FireSphere.h"

class Bullet : public IRenderable , public ICollidable //derieves from two interfaces
{
public:
	Bullet(GraphicsEngine* ge); 
	~Bullet();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	//ICollidable functions
	void Destroy(); //sets buet inactive
	Vector3 GetPosition() { return m_position; }; //rturns position
	float GetScale() { return m_scale; }	//returns scale
	
	void SetStartPosition(Vector3 pos, Vector2 dir); //sets start position and direcion of movement
	void Update(float dTime);	//updates bullet, called every frame

	bool IsActive(){ return m_isActive; } //return true if bullet is active

private:
	void StartFire(); //starts fire particle system
	//sphere creation functions
	void PushBackTriangle(Vertex &v1, Vertex &v2, Vertex &v3);
	Vertex GetMidPoint(Vertex &v1, Vertex &v2);
	void Subdivide(Vertex &v1, Vertex &v2, Vertex &v3, int depth);

	GraphicsEngine* m_engine = nullptr; //reference to engine 
	FireSphere* m_fire = nullptr;	//reference to child fire sphere object

	//rendering resources
	ID3D11Resource*				m_texture = nullptr;
	ID3D11ShaderResourceView*	m_textureView = nullptr;
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;

	//list of vertices where each consecutive three make a triangle
	vector<Vertex> m_listFaces; 

	//member variables
	Vector3 m_position; //position of bullet
	Vector2 m_direction;	//direction of movement
	float m_scale;	//bullet scale
	float m_speed; //movement speed
	float m_lifeSpan;	//lifetime
	float m_timeAlive;	//how long bullet is alive
	bool m_isActive;	//is bullet active

};