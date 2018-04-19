#pragma once

#include "GraphicsEngine.h"
#include "MazeGenerator.h"

//derives from three classes: IRenderable - for rendering, ICollidable - for collision detection, IEnemy for enemy list in mazeGenerator
class Squasher :public IRenderable, public IEnemy, public ICollidable
{
public:
	Squasher(GraphicsEngine* engine, MazeGenerator* maze);
	~Squasher();

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	//ICollidable functions
	void Destroy(){ return; } //this function does nothing for Squasher object
	Vector3 GetPosition(){ return m_position; }
	float GetScale(){ return m_scale; }
	//IEnemy
	void Update(float dt);
	void Delete(){ delete this; }
	//mutators
	void SetPosition(float x, float y, float z);
	void SetScale(float s);
	void UpdateRotation(Vector3 rot, float dTime);

private:
	//private functions
	//AI logic
	void ResetPatrolPoints();
	void PlanRouteToNextPatrolPoint();
	//private functions for sphere generation
	void PushBackTriangle(Vertex &v1, Vertex &v2, Vertex &v3);
	Vertex GetMidPoint(Vertex &v1, Vertex &v2);
	void Subdivide(Vertex &v1, Vertex &v2, Vertex &v3, int depth);
	//helper function that transforms maze tile coordinates into world units
	Vector2 ConvertToVector2(IntVec2 vec);

	// rendering resources
	ID3D11Buffer*				pVertexBuffer = nullptr;
	ID3D11Buffer*				pInstanceBuffer = nullptr;
	ID3D11Resource *			pTexture = nullptr;
	ID3D11ShaderResourceView *	pTextureView = nullptr;
	//references
	GraphicsEngine* m_engine = nullptr; //every object uses access to the engine
	MazeGenerator* m_maze = nullptr;	//access to maze needed for Djikstra

	vector<Vertex> m_listFaces; // list of vertices

	//transformation variables
	Vector3 m_position;
	Vector3 m_rotationAxis; //axis to rotate around
	float m_angle;			//rotation angle
	float m_scale;

	//AI
	// for patroling code
	vector<IntVec2> m_patrolPoints; // a list of current patrol points
	int m_nextPatrolPoint; // the next point on our patrol
	vector<IntVec2> m_route; // our current route - a route takes us from one patrol point to another and is calculated using Drijkstra
	unsigned int m_routeIndex; // our current index in the route, this would start and 0 and keep incrementing till we reach the last point in the route meaning we arrived

	IntVec2 m_posTile; // position of the enemy in maze coordinates for calculation purpose
	IntVec2 m_moveTarget; // their target in pixels - not maze coordinates
	IntVec2 m_moveVec; // direction enemy is currently moving
	Vector2 m_posVec2; //position in floats, world units

	
};