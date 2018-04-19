#include "stdafx.h"
#include "Game.h"


//define some maze properties
#define MAZE_SIZE_X 17 // these need to be an odd number of units large
#define MAZE_SIZE_Y 17
#define CUBE_SIZE 2 // from -1 to 1

Game::Game(GraphicsEngine* ge) //constructor
{
	m_engine = ge; //reference to graphics engine
}

Game::~Game() //destructor
{
	//delete maze objects & ramp and set to 0 their pointers
	delete m_level1; 
	m_level1 = nullptr;
	delete m_level2;
	m_level2 = nullptr;
	delete m_ramp;
	m_ramp = nullptr;

	//deleting player object
	m_engine->RemoveRenderableObject(m_player, true, false); //remove from renderable list
	delete m_player; //delete object
	m_player = nullptr;	//empty the pointer
	
}

void Game::GenerateScene() //function that creates game scene
{
	m_level1 = new MazeGenerator(m_engine, MAZE_SIZE_X, MAZE_SIZE_Y, CUBE_SIZE, 1, 2); //create maze level1
	m_level1->Initialise(); //call initalise on created maze

	m_level2 = new MazeGenerator(m_engine, MAZE_SIZE_X, MAZE_SIZE_Y, CUBE_SIZE, 2, 2); //create level2 maze
	m_level2->Initialise(); //call initalise on created maze

	//obtain calculated position for ramp allocation
	Vector3 pos = GetRampPosition();
	m_ramp = new Ramp(m_engine, pos); //create ramp object
	m_engine->AddRenderableObject(m_ramp, false, true); //add it to renderable object list, false for not casting shadows, true for semi-transparency
	m_engine->UpdateRampPosition(pos); //letting engine know where ramp is position for collision calculations

	//create GUI manager object
	m_UIManager = new UIManager(m_engine);
	m_UIManager->Initialise(); //initialise that object
	//create player object
	m_player = new Player(m_engine);
	m_engine->AddRenderableObject(m_player, true, false); //add it to renderable object list, true for casting shadows, flase for not using alpha
	Vector3 position = m_level1->GetStartPosition(); //get start position of from the maze where player should start
	m_player->SetPosition(position.x,position.y,position.z); //set player to that position
	isPlayerAlive = true; //set control bool to player is alive
	isGameWon = false;

	//create arrow object that direct player
	m_sign = new Sign(m_engine); 
	position = m_level2->GetEndPosition(); //get final destination position
	m_engine->AddRenderableObject(m_sign, false, true); //add sign to renderable object list, not casting shdows, is transparent
	m_sign->SetPosition(position.x, 0, position.z);	//position sign there

	return;
}

void Game::Tick() //called every frame from main.cpp
{
	float dTime = m_engine->GetDeltaTime(); //get delta time from the engine timer
	int playerLocation = GetPlayerLocation(); //request player current location
	
	//update sign passing player location
	UpdateDirections(playerLocation);
	
	//if player is alive
	if (isPlayerAlive)
	{
		m_engine->UpdatePlayerLocation(playerLocation); //letting engine know about player location
		m_player->Update(dTime, GetPlayerY()); //moves player, Y position is precalculated using player location in the world (see function for details)
		int health = m_player->GetHealth(); //checking player health
		m_UIManager->SetHealth(health); //updating health bar
		
		if (health <= 0) //if health is 0 (or less)
		{
			m_engine->RemoveRenderableObject(m_player, true, false); //remove player from rendering list
			m_player->Explode(); //start explosion
			isPlayerAlive = false; //set player to dead so this updates are not called
			m_timeInPause = 0; //start a pause timer
		}
		
		if (IsGameWon()) //check if player reached end
		{
			//this is handled similary to players death just player keeps being rendered and explosion is not called
			isPlayerAlive = false; //set player to dead so this updates are not called
			isGameWon = true;	//set game won state
			m_timeInPause = 0; //start a pause timer
			m_UIManager->SetGameEnd(true); //get "game won" sign
		}	
	}
	//if player is dead
	else
	{
		m_player->DeadUpdate(dTime); //some objects are dependent on the player and need updating even player is dead (hence the name)
		m_timeInPause += dTime; //update pause timer
		if (m_timeInPause > 5.0f) //if game was paused for 5 secs call reset the game
  			ResetGame();
	}

	//enemies and pickups keep updating no mater if player is alive or not
	m_level1->Update(dTime);
	m_level2->Update(dTime);
		
	return;
}

void Game::ResetGame() //function that resets player to it's start position
{
	int loc = GetPlayerLocation(); //get players current location
	/*
		NOTE: if player won ( player will respawn on level 2 again, so I need to fix this here )	
	*/
	
	if (isGameWon) //if reset is happening because player has won the game
	{
		loc = 1; //set location to level 1
		isGameWon = false; //reset game won bool
	}
	else //reset happened because player died ( only other option )
	{
		//I have to add player objec only in the case he died ( if player wins, i dont remove it from render list )
		m_engine->AddRenderableObject(m_player, true, false); //add player back to renderable list
	}
		
	
	
	Vector3 startPosition; //vector variable to hold players position to spawn
	
	if (loc == 2) //if player died on level2 it will respawn in the start of level2
		startPosition = m_level2->GetStartPosition(); //get level2 start position
	else //if player died on level1 or on the ramp respawn player on level 1
		startPosition = m_level1->GetStartPosition(); //get level1 start position
	m_player->SetPosition(startPosition.x, startPosition.y, startPosition.z); //set player to respawn position
	
	isPlayerAlive = true; //set control bool to player is alive
	
	m_player->SetHealth(100); //set player to full health
	m_UIManager->SetGameEnd(false); //deactivate game end text ( if it was active )

	return;
}

bool Game::IsGameWon() //checks if player has reached the destination tile
{
	//need to check if player has collided with the end square
	//get position of the players destination
	Vector3 end = m_level2->GetEndPosition();
	//need player position as well
	Vector3 pos = m_player->GetPosition();
	//now simple collision arithmetics ( if player is 0.5 units away from destination tiles center )
	if (fabs(end.x - pos.x) < 0.5f && fabs(end.z - pos.z) < 0.5f)
		return true; //collided, returns true

	return false;
}

//Getters
Vector3 Game::GetRampPosition() //returns the position of the ramp
{
	IntVec2 midPos = m_level1->GetRampTile() + IntVec2(1, 0); //move 1 unit away from position passed by maze(which is just the strt of the rmp)
	Vector3 ret;
	ret.x = (float)midPos.x * CUBE_SIZE; //multiply by tile size
	ret.y = 0.0f;
	ret.z = (float)midPos.y * CUBE_SIZE; 

	return ret;
}

int Game::GetPlayerLocation() // returns int, 0 is for ramp, 1 - level1, 2 - level2
{
	Vector3 pos = m_player->GetPosition(); //get player position
	//as the scene I am creating is quite linear i can track players psition by its X coordinate
	float edge = MAZE_SIZE_X * CUBE_SIZE - 1.0f; // level1 edge on X axis
	Vector3 offset = m_level2->GetOffset(); //offset used to create space for ramp
	
	if (pos.x <= edge) // if player's X is less than maze size on X axis
		return 1; //one stands for level1 , described in game.h
	else if (pos.x >= offset.x - 1.0f) // player has passed start of level2
		return 2;	//2 stands for level2
	else
		return 0; //in all other cases player  must be on the ramp
}

float Game::GetPlayerY()
{
	float playerScale = m_player->GetScale(); //get player scale
	int location = GetPlayerLocation(); //get player location
	Vector3 offset = m_level2->GetOffset(); //offset used to create space for ramp

	if (location == 1) //if player is on level1
		return  playerScale - 0.5f; // slightly above the ground ( ground is located at -1 on Y axis
	else if (location == 2) //if player is on level2
		return playerScale + offset.y - 0.5f; //same as on level 1 but adding offset as level 2 is located higher
	else //if player is on ramp
	{
		//for ramp Y calculation is lightly more difficult because  have to lineary interpolate from level1 to level2 Y positions
		Vector3 pos = m_player->GetPosition();
		//to calculate Y when on ramp I need to use interpolation
		float rampStart = MAZE_SIZE_X * CUBE_SIZE - 1.0f; // ramp starts on level1 egde
		float rampEnd = offset.x - 1.0f; // offset indicates where level2 starts
		float rampSize = rampEnd - rampStart; // getting the size of the ramp ( should be 2 * CUBE_SIZE )
		float distance = pos.x - rampStart; // how far on the ramp player has traveled
		//now calculating variable that indicates the position on the ramp in ramp units ( example 0.5 indicates we are in the middle of the ramp)
		float value = distance / rampSize; 
		//on Y axis: Ramp starts at 0.0 and ends at offset.y , which means ramp size = offset.y
		float ret = playerScale + 2.0f * value - 0.5f; //value is multiplied by 2.0 because 2.0 is the height of the ramp
		return ret; 
	}
}

void Game::UpdateDirections(int playerLocation) //updates the location of the arrow sign
{
	if (playerLocation == 1) //if player is on level1 arrow directs player to the ramp
	{
		Vector3 position = GetRampPosition();
		m_sign->SetPosition(position.x, 1, position.z);
	}
	else //otherwise arrow directs player to the end tile on level2
	{
		Vector3 position = position = m_level2->GetEndPosition();
		m_sign->SetPosition(position.x, 3, position.z);
	}

	m_sign->Update(); //call update on sign object

	return;
}