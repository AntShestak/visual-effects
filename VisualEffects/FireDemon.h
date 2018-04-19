#pragma once

#include "MazeGenerator.h"
#include "Bullet.h"
#include "FireSphere.h"

class FireDemon :public IRenderable, public IEnemy  //derives from two interfaces
{
public:
	FireDemon(GraphicsEngine* ge, MazeGenerator *maze); //constructor
	~FireDemon();	//destructor

	// IRenderable functions which we have to implement
	void Initialise(DirectXWrapper *wrapper); // called once when added to the wrapper
	void Render(DirectXWrapper *wrapper, int pass); // called every frame
	Vector3 GetPosition(){ return m_position; }
	//IEnemy functions
	void Update(float deltaTime);  //updates object
	void Delete(){ delete this; }	//deletes itself
	//setters
	void SetPosition(float x, float y, float z); // sets position
	void SetScale(float x);
	void SetRotation(int x, int z); //sets rotation angle, which is dependent on current movement vector
	void SetLevelOffset(Vector3 offset); //this is used to adjust position correctly

private:
	//private functions
	//sphere generation
	void PushBackTriangle(Vertex &v1, Vertex &v2, Vertex &v3);
	Vertex GetMidPoint(Vertex &v1, Vertex &v2);
	void Subdivide(Vertex &v1, Vertex &v2, Vertex &v3, int depth);
	//route finding
	void ResetPatrolPoints();
	void PlanRouteToNextPatrolPoint();
	//shooting
	void DestroyBullet(Bullet* bullet); //destroys the bullet
	bool CanSeePlayer(); //returns true if enemy can see the player
	//function that starts a fire particle system
	void StartFire();
	//my helper function that transforms maze tile coordinates into world units
	Vector2 ConvertToVector2(IntVec2 vec); 
	//rotation helper function
	float ConvertToRadians(float degrees);
	
	//references
	MazeGenerator* m_maze = nullptr;
	GraphicsEngine* m_engine = nullptr;
	Bullet* m_bullet = nullptr;
	FireSphere* m_fire = nullptr;
	//rendering resources
	ID3D11Buffer*				pVertexBuffer = nullptr;
	ID3D11Buffer*				pInstanceBuffer = nullptr;
	ID3D11Resource *			pTexture = nullptr;
	ID3D11ShaderResourceView *	pTextureView = nullptr;
	
	//lists
	vector<Bullet*> m_listBullets; //list of bullets controled by this enemy
	vector<Vertex> m_listFaces; // list if vertices, where every three consecutive vertices make a triangle
	//transformation variables
	float m_scale;
	Vector3 m_position;
	Vector3 m_levelOffset;
	float m_angle;
	
	// AI variables used for calculations
	//lists
	vector<IntVec2> m_patrolPoints; // a list of patrol points (tiles) 
	int m_nextPatrolPoint; // the next point on patrol
	vector<IntVec2> m_route; // list of tiles that lead from one tile to another tile from patrol point lst ( route is calculated in mazeGenerator )
	unsigned int m_routeIndex; // current index in the route

	IntVec2 m_posTile; // position in maze units (ints)
	IntVec2 m_moveTarget; // target positon in maze units
	IntVec2 m_moveVec; // direction enemy is currently moving
	Vector2 m_posVec2;	//position in world units, used for calculations

};