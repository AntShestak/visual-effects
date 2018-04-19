#include "stdafx.h"
#include "GraphicsEngine.h"

#include "DirectXWrapper.h"


GraphicsEngine::GraphicsEngine()
{
	//nothing to do in constructor as all pointers are nulled in header
}

GraphicsEngine::~GraphicsEngine()
{
	//delete directX object
	m_directX->DeleteDirectXWrapper(m_directX);
	//delete shader classes
	delete m_depthShader;
	delete m_shadowShader;  
	delete m_uiShader;
	delete m_particleShader;
	//delete other object created
	delete m_camera;
	delete m_light;
	delete m_timer;
}

bool GraphicsEngine::Initialise(HWND hwnd, HINSTANCE hinst)
{
	//create directXwrapper
	m_directX = DirectXWrapper::GetDirectXWrapper(hwnd, hinst);
	//create all shaders
	m_depthShader = new DepthShader(m_directX);
	m_shadowShader = new ShadowShader(m_directX);
	m_uiShader = new UIShader(m_directX);
	m_particleShader = new ParticleShader(m_directX);
	//assign windows handle passed from main.cpp
	m_windowHandle = hwnd;
	//create light object
	m_light = new Light();
	m_light->Initialise(); //initialise light
	//create camera
	m_camera = new Camera();
	m_camera->Initialise(m_directX, hwnd); //camera requires handle for window dimensions update
	//create timer
	m_timer = new Timer();
	m_timer->Initialize(); //initialise timer
	
	return true;
}

void GraphicsEngine::SetWorldTransform(XMMATRIX &wm)
{
	//this function simply updates the world matrix for next object to render
	worldMatrix = wm;
	return;
}

//render lists control functions
void GraphicsEngine::AddRenderableObject(IRenderable* pObject, bool isCaster, bool isAlpha)
{
	//this function directs an object to the relevant list for rendering
	if (isCaster) 
		m_directX->AddCasterObject(pObject);	//list of objects that cast shadows
	else if (isAlpha)
		m_directX->AddAlphaObject(pObject);		//list of objects that use transparency
	else 
		m_directX->AddNonCasterObject(pObject); //list of objects that don't cast shadows & don't use transparency

	return;
}

void GraphicsEngine::RemoveRenderableObject(IRenderable* pObject, bool isCaster, bool isAlpha)
{
	//removes object from relevant list
	//for more details see function above
	if (isCaster)
		m_directX->RemoveCasterObject(pObject);
	else if (isAlpha)
		m_directX->RemoveAlphaObject(pObject);
	else
		m_directX->RemoveNonCasterObject(pObject);

	return;
}

void GraphicsEngine::AddUIObject(IRenderable* pObject)
{
	//adds a GUI object to be redered
	m_directX->AddUIObject(pObject);
}

void GraphicsEngine::AddParticleObject(IRenderable* pObject)
{
	//add an object to the list of particle objects to be rendered
	m_directX->AddParticleObject(pObject);
}

void GraphicsEngine::RemoveParticleObject(IRenderable* pObject)
{
	//removs an object from particle list
	m_directX->RemoveParticleObject(pObject);
}

//rendering control functions
void GraphicsEngine::RenderShadowMap()	//this function gets called by an object in the 1st rendering pass
{
	//obtain light view & projection matrices from light class
	XMMATRIX lview = m_light->GetViewMatrix();
	XMMATRIX lprojection = m_light->GetProjectionMatrix();
	//pass these to shader along with world matrix stored (is the world matrix for the next object to be rendered)
	m_depthShader->Render(worldMatrix,lview,lprojection); //depth shader class sets shaders to render to depth texture

	return;
}

void GraphicsEngine::RenderToScreen(ID3D11ShaderResourceView* texture) //this function is called by the object to be rendered in the 2nd rendering pass
{
	//obtain camera view and projection matrices from camera class
	XMMATRIX cView = m_camera->GetViewMatrix();
	XMMATRIX cProjection = m_camera->GetProjectionMatrix();
	//obtain light view & projection matrices from light class
	XMMATRIX lView = m_light->GetViewMatrix();
	XMMATRIX lProjection = m_light->GetProjectionMatrix();
	//pass these matrices to shader along with world matrix for the object to be rendered
	//also shadow shader requires texure view & light reference to obtain some light variables
	m_shadowShader->Render(worldMatrix, cView, cProjection,lView,lProjection, texture, m_light);

	return;
}

void GraphicsEngine::RenderUI(ID3D11ShaderResourceView* texture)
{
	//pass objects world matrix and texture to the relevant shader
	m_uiShader->Render(worldMatrix, texture);
}

void GraphicsEngine::RenderParticles(ID3D11ShaderResourceView* texture)
{
	//obtain matrices from camera
	XMMATRIX camView = m_camera->GetViewMatrix();
	XMMATRIX camProj = m_camera->GetProjectionMatrix();
	//pass these matrices along with texture and world matrix to render particle system
	m_particleShader->Render(worldMatrix, camProj, camView, texture);

	return;
}

//Light control function
void GraphicsEngine::MoveLight(Vector3 playerPos)
{
	//this functions moves light, called from player.cpp
	//min & max position to clamp light movement
	float min = 16.0f;
	float max = 50.0f;
	//obtain light pos & look at position
	Vector3 pos = m_light->GetPosition();
	Vector3 look = m_light->GetLookAt();
	//calculate offset for obtained variables
	Vector3 offset = pos - look;
	
	look = playerPos; //set the look at to players position
	look.y = 0.0f; //excepth for the Y axis, so light doesn't move on Y axis
	//now need to clamp x & z position in a set range
	if (look.x < min)
		look.x = min;
	else if (look.x > max)
		look.x = max;
	//now clamp on Z axis
	if (look.z < min)
		look.z = min;
	else if (look.z > max)
		look.z = max;
	//calculate new position
	pos = look + offset;
	//update lights properties
	m_light->SetPosition(pos.x, pos.y, pos.z); //set new position
	m_light->SetLookAt(look.x, look.y, look.z); //set new look at position
	m_light->GenerateViewMatrix();	//now recalculate view matrix

	return;
}

//tick is called every frame from main.cpp
void GraphicsEngine::Tick()
{
	m_directX->Render(); //rendering everything that needs to be rendered
	
	return;
}
//Getters
float GraphicsEngine::GetDeltaTime()
{
	//retrieves delta time from timer class
	return m_timer->GetTime();
}

XMMATRIX GraphicsEngine::GetCameraRotationY()
{
	//returns rotation matrix for camera's rotation around Y axis (used for player & objects that need to face camera)
	return m_camera->GetRotationY();
}

//collision control functions
void GraphicsEngine::AddWallCollider(Vector3 pos,  int level) //pass position of the wall, and location in game scene
{
	//this way I have separate list of walls for every level and I don't need to go through all the walls in the game world
	//to determine collision, as long as I know player location
	if (level == 1) //if wall is on level1
	{
		m_listWalls1.push_back(pos); //add it to level1 list
	}
	else
		m_listWalls2.push_back(pos); //in other case add it to level2 list (as there are only 2 levels)

	return;
}

void GraphicsEngine::AddPickUpCollider(ICollidable* pObject)
{
	m_listPickUps.push_back(pObject); //add to the list
	return;
}
void GraphicsEngine::RemovePickUpCollider(ICollidable* pObject)
{
	//iterae through the list to find requested object
	for (unsigned int i = 0; i < m_listPickUps.size(); i++)
	{
		if (m_listPickUps[i] == pObject)
		{
			//erase the object 
			m_listPickUps.erase(m_listPickUps.begin() + i);
			break;
		}
	}

	return;
}

void GraphicsEngine::AddEnemyCollider(ICollidable* pObject)
{
	m_listEnemies.push_back(pObject); //adding enemy to the list of enemies's colliders
	return;
}
void GraphicsEngine::RemoveEnemyCollider(ICollidable* pObject)
{
	//iterate through the list to find required object
	for (unsigned int i = 0; i < m_listEnemies.size(); i++)
	{
		if (m_listEnemies[i] == pObject)
		{
			//erase the object from the list
			m_listEnemies.erase(m_listEnemies.begin() + i);
		}
	}

	return;
}

bool GraphicsEngine::CheckPickUpCollision(float x, float z, float r) //returns true if collided (x & z are the position, R is radius of object that is being checked for collision)
{
	//all picks ups are the same size so I obtain scale of the first one and use it for calculations
	float scale = m_listPickUps[0]->GetScale();
	//now I need pickUp radius. 
	//I have scale, and all my cube primitives have base size of 2 (-1 to 1), also to obtain radius I have to divide by 2.
	//this means I have to multiply by 2 and divide by 2 which retun scale unchanged
	float objectRadius = scale; 
	//now calculating offset which is a distance allowed between picked objects and players centers
	float offset = objectRadius + r;
	//going through all pickUps
	for (unsigned int i = 0; i < m_listPickUps.size(); i++)
	{
		//obtaining objects position
		Vector3 pos = m_listPickUps[i]->GetPosition();
		//if player is closer to the objects( on both axis!) than offset - they collided
		if ((fabs(pos.x - x) < offset) && (fabs(pos.z - z) < offset))
		{
			//first I destroy pick up
			m_listPickUps[i]->Destroy(); 
			//and return true for collision
			return true;
		}
	}
	//otherwise return false;
	return false;
}

bool GraphicsEngine::CheckWallCollision(float x, float z, float r)	//passing postion on x, z axis and a radius
{
	float wallRadius = 1.0f; //I know this as long as walls are not scaled on x or z axis ( which would break maze generator anyway)
	
	if (m_playerLocation == 1) //if player is on leve1
	{
		for (unsigned int i = 0; i < m_listWalls1.size(); i++) //go through level1 list of walls
		{
			float offset = r + wallRadius; //this describes minimum difference in position allowed between player and the wall
			//this is simple calculation: check if distance (abs, to be always positive value) between player position
			//and wall position are less then offset value, and return true if it is less, which means collision
			if ((fabs(m_listWalls1[i].x - x) < offset) && (fabs(m_listWalls1[i].z - z) < offset)) return true;
		}
	}
	else if (m_playerLocation == 2) //if player is on level2 it's the same algorhytm, just using walls2 list
	{
		for (unsigned int i = 0; i < m_listWalls2.size(); i++) 
		{
			float offset = r + wallRadius; 
			if ((fabs(m_listWalls2[i].x - x) < offset) && (fabs(m_listWalls2[i].z - z) < offset)) return true;
		}
	}
	else //the last option is player is on the ramp
	{
		//here calculation is slightly different
		float offset = wallRadius - r; // here offset describes allowed distance from center of ramp to any direction (on z axis)
		//if player moves further from center than offset is allowing returns collision
		if ((z >= m_rampPosition.z + offset) || (z <= m_rampPosition.z - offset)) return true;
	}
	//if no collision detected
	return false;
}
bool GraphicsEngine::CheckWallCollision(float x, float z, float r, int location)
{
	//this is overloaded wall collision function. It is used when we need to check for collision with walls not taking into count player's current location
	//therefore I pass location into this function to be sure which list I need to check against
	float wallRadius = 1.0f; //I know this as long as walls are not scaled on x or z axis ( which would break maze generator anyway)
	//currently this function is only used on level2, so only option is location = 2, however this could be easily expanded if needed
	if (location == 2)
	{
		//go through list of level2 walls
		for (unsigned int i = 0; i < m_listWalls2.size(); i++)
		{
			float offset = r + wallRadius; //offset is distance between object centers if they are next to each other
			if ((fabs(m_listWalls2[i].x - x) < offset) && (fabs(m_listWalls2[i].z - z) < offset)) return true; //return true if collided
		}
	}
	//no collision detected
	return false;
}

bool GraphicsEngine::CheckEnemyCollision(float x, float z, float r) 
{
	//it's only few enemies in the game o I didn't divide this into separate lists
	//iterate through enemies
	for (unsigned int i = 0; i < m_listEnemies.size(); i++)
	{
		//first need to calculate offset
		float enemyRadius = m_listEnemies[i]->GetScale(); //because objects base size is 2 I would have to * by 2 (base size of primitives is 2), then / 2 to get radius so I leave it as it is
		float offset = enemyRadius + r;
		//get enemy positon 
		Vector3 pos = m_listEnemies[i]->GetPosition();
		//now check for collision
		if ((fabs(pos.x - x) < offset) && (fabs(pos.z - z) < offset))
		{
			m_listEnemies[i]->Destroy(); //calling destroy function on object collided (this function is empty for level1 enemy, its coded for bullets)
			return true;
		}
	}
	return false; //if no collisions detected
}


