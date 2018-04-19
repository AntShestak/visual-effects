#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"

//class that renders an arrow object to help direct player to the destination
class Sign :public IRenderable //derives from IRenderable interface as any renderable object
{
public:
	//constructor & destructor
	Sign(GraphicsEngine* engine);
	~Sign();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }
	//updates sign
	void Update();
	//setters
	void SetPosition(float x, float y, float z); 
	
private:
	//engine reference
	GraphicsEngine* m_engine;
	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	//variables
	float m_scale;
	Vector3 m_position; //final position for atrix calculations
	Vector3 m_startPosition; //start position is used for position calculations
	Vector3 m_targetPosition; //players position stored for calculations
};