#pragma once

#include "IRenderable.h"
#include "GraphicsEngine.h"

//class that renders a bar on the screen
class HealthBar :public IRenderable
{
public:
	HealthBar(GraphicsEngine* ge, bool isBackSide); //pass bool into constructor to understand if it is back or front side of health slider
	~HealthBar();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return Vector3(m_position.x, m_position.y, 0.0f); } //returns position

	void SetHealth(float percent); //takes in float 1.0 is full heath

private:

	GraphicsEngine* m_engine = nullptr;
	//rendering resources
	ID3D11Buffer*				m_vertBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	//member variables
	Vector2 m_startPosition;	//starting position of the bar
	Vector2 m_startScale;	//starting scale of the bar
	Vector2 m_position;		//current position of the bar
	Vector2 m_scale;		//current scale
	
	float m_depth; //variable to control front bar to be rendered on top of the red back bar
	float m_percent; //percentage of health left
	bool m_isBackSide;//helper function to know which part of slider this is

};