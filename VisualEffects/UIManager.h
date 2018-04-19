#pragma once
#include "HealthBar.h"
#include "GameEndBoard.h"
//this class is responsible for taking care of all user interface objects
class UIManager
{
public:
	UIManager(GraphicsEngine* engine); //constructor 
	~UIManager();

	void Initialise();
	void SetHealth(int health); //passing amount of health
	void SetGameEnd(bool isEnd); //calls game end text if passed true
private:
	GraphicsEngine* m_engine = nullptr; //reference to the game engine

	vector<IRenderable*> m_objectList; //list of UI bjects

	HealthBar* healthBack = nullptr;	//health bar back(red bar)
	HealthBar* healthFront = nullptr;	//health bar front (green bar)
	GameEndBoard* gameWonBoard = nullptr;	//game won text

};