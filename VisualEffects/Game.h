 #pragma once
//includes for references
#include "GraphicsEngine.h"
#include "MazeGenerator.h"
#include "UIManager.h"
#include "Player.h"
#include "Sign.h"
#include "Ramp.h"
//class that generates & controls game scene
class Game
{
public: 
	Game(GraphicsEngine* ge); //constructor
	~Game();	//destructor

	void GenerateScene(); //generates scene
	void Tick();	//updates scene every frame called form main.cpp
	void ResetGame(); //resets game when the player dies
	void UpdateDirections(int location); //updates arrow object that directs player to destination

	//Getters
	 Vector3 GetRampPosition(); //returns
	int GetPlayerLocation(); // returns int, 0 is for ramp, 1 - level1, 2 - level2
	float GetPlayerY(); //return player's position on Y axis, used by player object
	bool IsGameWon(); //check if player as reached final destination

private:
	GraphicsEngine* m_engine = nullptr; //reference to engine
	//game object references
	Player*	m_player = nullptr; //player object
	UIManager* m_UIManager = nullptr;	//GUI manager 
	MazeGenerator* m_level1 = nullptr;	//level 1 maze
	MazeGenerator* m_level2 = nullptr;	//level 2 maze
	Ramp* m_ramp = nullptr; //ramp object
	Sign* m_sign = nullptr;	//arrow sign object used as adirection

	//member variables
	bool isPlayerAlive;		//is true when layer is alive
	bool isGameWon;		//it's true when player won
	float m_timeInPause;	//time game stays in pause when player dies

};