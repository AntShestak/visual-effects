#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"

//this class is an object with "Game Won" text
class GameEndBoard :public IRenderable
{
public:
	GameEndBoard(GraphicsEngine* engine);
	~GameEndBoard();

	// IRenderable functions which we I have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }

	void SetPosition(float x, float y, float z); // sets position of the object
	void SetScale(float s);			//sets scale of the bject

private:
	
	GraphicsEngine* m_engine = nullptr; //engine reference

	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	//member variables
	float m_scale;
	Vector3 m_position;
};