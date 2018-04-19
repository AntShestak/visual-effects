#pragma once

#include "IRenderable.h"

#include "GraphicsEngine.h"

//class that renders a floor tile
class FloorTile :public IRenderable		//derives from IRebnderable interface required for all rederable objects
{
public:
	//construtor takes stage parameter that explains which level to generate level 1 or 2
	FloorTile(GraphicsEngine* engine, int stage); 
	~FloorTile();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper,int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }

	void AddInstance(Vector3 pos); //adds fllor tile at pos
	//setters
	void SetScale(float s);
	
private:
	GraphicsEngine* m_engine; //engine reference

	//rendering resources
	ID3D11Buffer*				m_vertexBuffer = nullptr;
	ID3D11Buffer*				m_instanceBuffer = nullptr;
	ID3D11Resource *			m_texture = nullptr;
	ID3D11ShaderResourceView *	m_textureView = nullptr;

	vector<InstanceData> m_listInstances;	//list to hold instance data

	//depending on level some some parameters change ( different texture is used )
	int m_levelNum; //gets assigned in constructor
	//tile properties
	float m_scale;
	Vector3 m_position;

};