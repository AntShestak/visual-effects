#include "stdafx.h"
#include "UIManager.h"

UIManager::UIManager(GraphicsEngine* engine)
{
	m_engine = engine; //reference the enginee
}

UIManager::~UIManager()
{
	//delete all objects & null their pointers
	delete healthBack;
	healthBack = nullptr;
	delete healthFront;
	healthFront = nullptr;
	delete gameWonBoard;
	gameWonBoard = nullptr;

}

void UIManager::Initialise()
{
	//Initialise function creates all the objects
	healthBack = new HealthBar(m_engine, true);	//create red health bar
	m_engine->AddUIObject(healthBack);	//add it the renderable list
	healthBack->SetHealth(1.0f); //set it to full size
	healthFront = new HealthBar(m_engine, false); //create green health bar
	m_engine->AddUIObject(healthFront); //add it to the renderable object list. 
	healthFront->SetHealth(1.0f); //set to full health
	gameWonBoard = new GameEndBoard(m_engine); //create game won text
	gameWonBoard->SetScale(0.3f);	//set appropriate scale
	gameWonBoard->SetPosition(2.0f, 0.0f, 0.0f); //set position (outside of the screen bounds)
	m_engine->AddUIObject(gameWonBoard); //add to renderable list

}

void UIManager::SetHealth(int health) //I pass health as integers from Game.cpp
{
	float var = (float)health / 100.0f; // turning amount into decimals
	
	healthFront->SetHealth(var); //set the size of the green bar

	return;
}

void UIManager::SetGameEnd(bool isEnd) //simple function if passed true positions object on the screen, if false - positions object outside of the screen bounds
{
	if (isEnd)
		gameWonBoard->SetPosition(0.0f, 0.0f, 0.0f); //middle of the screen
	else
		gameWonBoard->SetPosition(2.0f, 0.0f, 0.0f); //outside of the screen bounds

	return;
}