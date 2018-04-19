#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"

class Ramp : public IRenderable //derives from IRenderable interface
{
public:
	Ramp(GraphicsEngine* ge, Vector3 pos); //costructor takes engine reference & position vector
	~Ramp();

	// IRenderable interface functions
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }

private:
	// Rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource*				m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	GraphicsEngine* m_engine = nullptr; //reference to egine
	Vector3 m_position;	//position
};