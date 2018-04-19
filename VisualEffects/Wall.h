#pragma once

#include "IRenderable.h"

#include "GraphicsEngine.h"

//class that renders a cube that represents the wall
class Wall :public IRenderable
{
public:
	Wall(GraphicsEngine* engine, int stage); //contructor takes in stage parameter ( which is level number where wall is generated )
	~Wall();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper,int pass); // called every frame
	Vector3 GetPosition(){ return m_position; } //this function need to be here as part of IRenderable interface, however is not gonna work, because m_position is not used

	void AddInstance(Vector3 pos); //used to add instances to list
	//set cube scale
	void SetScale(float x, float y, float z);

private:
	//private function that calculates a nrmal for a face with 3 vertices
	Vector3 GetNormal(Vector3 &vertex1, Vector3 &vertex2, Vector3 &vertex3);

	//engine reference
	GraphicsEngine* m_engine = nullptr;

	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	vector<InstanceData> instances; //list to hold instance data
	//depending on level some some parameters change ( different texture is used )
	int m_levelNum;	//gets assigned in constructor
	//main prperties 
	Vector3 m_position;
	Vector3 m_scale;

};