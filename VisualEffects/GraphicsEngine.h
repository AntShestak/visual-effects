#pragma once

//interfaces used mainly to group objects together
#include "ICollidable.h"
#include "IEnemy.h"
//include shader classes
#include "DepthShader.h"
#include "ShadowShader.h"
#include "UIShader.h"
#include "ParticleShader.h"
//include camera for reference
#include "Camera.h"
//include timer for deltaTime calculation
#include "Timer.h"

//structure for collidable objects
struct CollidableObject
{
	Vector3 position;
	float radius;
};
//class that is responsible for rendering & some physics 
class GraphicsEngine
{
public:
	//required overload for new
	void* operator new(size_t i)
	{
		return _aligned_malloc(i, 16);
	}
	//required overload for delete
	void operator delete(void* p)
	{
		_aligned_free(p);
	}
	
	GraphicsEngine();//constructor
	~GraphicsEngine(); //destructor

	bool Initialise(HWND hwnd, HINSTANCE hinst); //initialise is called from main.cpp
	void Tick();	//tick is called from main.cpp every frame
	
	void SetWorldTransform(XMMATRIX &worldMatrix); //sets world transforms, called from every object when it gets rendered
	//control functions of multiple rendering lists, each adds or removes from particular list of objects
	void AddRenderableObject(IRenderable* pObject, bool isCaster, bool isAlpha);
	void RemoveRenderableObject(IRenderable* pObject, bool isCaster, bool isAlpha);
	void AddUIObject(IRenderable* pObject);
	void AddParticleObject(IRenderable* pObject);
	void RemoveParticleObject(IRenderable* pObject);
	//render functions
	void RenderShadowMap(); //takes number of vertices
	void RenderToScreen(ID3D11ShaderResourceView* texture);
	void RenderUI(ID3D11ShaderResourceView* texture);
	void RenderParticles(ID3D11ShaderResourceView* texture);
	
	//functions for collision controls
	void AddWallCollider(Vector3 position, int level);	//adds a wall to the list reevant list of walls
	bool CheckWallCollision(float x, float z, float radius);	//check for collision with wall on position passed
	bool CheckWallCollision(float x, float z, float radius, int location); //this is overload to be used by bullet mainly (and in some other cases)
	void AddPickUpCollider(ICollidable* pickUp); //adds a pick up object to the list of pickUp pointers, usin ICollidable interface
	void RemovePickUpCollider(ICollidable* pickUp); //remove a pickup from the list after its picked
	bool CheckPickUpCollision(float x, float z, float r); //checks for collisions with pickUps
	void AddEnemyCollider(ICollidable* pObject); //adds an enemy object to the list of enemies's colliders
	void RemoveEnemyCollider(ICollidable* pObject); // removes enemy collider from the list
	bool CheckEnemyCollision(float x, float z, float r); //this functions check for collisions with enemies
	
	//light control function
	void MoveLight(Vector3 position);
	
	//Setters
	void UpdatePlayerLocation(int location){ m_playerLocation = location; } //this function tells engine where player is located, used for collision checks ( values are described below)
	void UpdatePlayerPosition(Vector3 position){ m_playerPosition = position; } //using this function engine gets updated on players position
	void UpdateRampPosition(Vector3 pos){ m_rampPosition = pos; }	//using this function engine gets aupdated on ramp's position

	//Getters
	Camera* GetCamera(){ return m_camera; } //return reference to the camera
	XMMATRIX GetCameraRotationY(); //returns rotation matrix of camera's rotation around Y axis
	Vector3 GetPlayerPosition(){ return m_playerPosition; } //returns player's position
	int GetPlayerLocation(){return m_playerLocation;}		//return player's location
	float GetDeltaTime();	//returns delta time
	
private:
	//object references
	DirectXWrapper* m_directX = nullptr;
	DepthShader* m_depthShader = nullptr;
	ShadowShader* m_shadowShader = nullptr;
	UIShader* m_uiShader = nullptr;
	ParticleShader* m_particleShader = nullptr;
	Camera* m_camera = nullptr;
	Light* m_light = nullptr;
	Timer* m_timer = nullptr;
	
	HWND m_windowHandle;	//window handle

	//matrices for calculations
	__declspec(align(16))
	XMMATRIX				worldMatrix ;
	XMMATRIX				WVP;
	XMMATRIX				lightWVP;

	//player position & location , engine keeps track of this so any object could access these
	Vector3 m_playerPosition;
	int m_playerLocation; //variable that describes player location ( 0 means player is on ramp, 1 - level1, 2 - leve2)
	//collision variables
	Vector3 m_rampPosition; //position of the ramp for collision calculations
	vector<Vector3> m_listWalls1; // this is list of walls on level 1 , need only their positions
	vector<Vector3> m_listWalls2; //list of walls on level2
	vector<ICollidable*> m_listPickUps; //list of pickup objects, using ICollidable interface
	vector<ICollidable*> m_listEnemies; //list of enemy objects, using ICollidable interface

};