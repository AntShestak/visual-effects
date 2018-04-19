// Main.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "Main.h"
#include "WindowsWrapper.h"
#include "GraphicsEngine.h"
#include "Game.h"

//main function
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	WindowsWrapper *window = new WindowsWrapper(1280, 720, L"Visual Effects", L"WindowsWrapper"); //create a window 
	
	GraphicsEngine* engine = new GraphicsEngine(); //create an engine
	bool result = engine->Initialise(window->GetHwnd(), GetModuleHandle(NULL)); //pass window handle to engine, for fps output
	
	Game *game = new Game(engine); //create new game class
	game->GenerateScene();	//generate the scene

	//the main loop
	do
	{
		//tick all objects every frame
		window->Tick();
		game->Tick();
		engine->Tick();
		//no frame rate cap is required as I use timing
		
	} while (!window->QuitRequested());
	
	//if quit was requested and loop exits need to clean up all the objects
	delete game;
	game = nullptr;
	delete engine;
	engine = nullptr;
	delete window;
	window = nullptr;

	return 0;
}

