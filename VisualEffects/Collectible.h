#pragma once
//include interfaces
#include "IRenderable.h"
#include "ICollidable.h"

#include "GraphicsEngine.h"

class Collectible : public IRenderable, public ICollidable //derives from two interface onr for rendering, other for collision detection
{
public:
	Collectible(GraphicsEngine* engine);
	~Collectible();
	//IRenderable functions
	void Initialise(DirectXWrapper* wrapper); //called once when added to renderable list
	void Render(DirectXWrapper* wrapper, int pass);	//called every frame
	//ICollidable functions
	void Destroy(); //sets object inactive after collision
	
	void Update(float dTime); //updates is called every frame

	//setters
	void SetActive(bool isActive){ m_isActive = isActive; }
	void SetPosition(Vector3 position);
	void SetScale(float scale);
	//Getters
	Vector3 GetPosition(){ return m_position; }
	float GetScale(){ return m_scale; } //returns objects scale
	bool IsActive(){ return m_isActive; } //returns if object is active

private:
	//private function that calculaes normal
	Vector3 CalculateNormal(Vector3 &vert1, Vector3 &vert2, Vector3 &vert3);
	
	GraphicsEngine* m_engine = nullptr; //engine reference
	
	// rendering resources
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView*	m_textureView = nullptr;
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;

	//member varibles
	Vector3 m_position; //position
	float m_scale;	
	float m_angle;	//angle for rotation
	bool m_isActive;
};