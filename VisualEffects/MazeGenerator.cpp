#include "stdafx.h"
#include "MazeGenerator.h"
//include enemy objects
#include "Squasher.h"
#include "FireDemon.h"


#include <time.h>		//time is used as a random seed
#include <algorithm> //used for random_shuffle line:60

//operator overloads
IntVec2 operator+ (const IntVec2& V1, const IntVec2& V2) { IntVec2 ret(V1.x, V1.y); return ret += V2; }
IntVec2 operator- (const IntVec2& V1, const IntVec2& V2) { IntVec2 ret(V1.x, V1.y); return ret -= V2; }
IntVec2 operator* (const IntVec2& V, int S) { IntVec2 ret(V.x, V.y); return ret *= S; }
IntVec2 operator* (int S, const IntVec2& V) { IntVec2 ret(V.x, V.y); return ret *= S; }

MazeGenerator::MazeGenerator(GraphicsEngine* ge,int wMaze, int hMaze, int size, int stage, int enemies)
{
	//assiging variables
	m_engine = ge;		//engine reference
	m_tileSize = size;	//size of the tile
	m_width = wMaze;	//maze dimensions
	m_height = hMaze;
	m_level = stage;	//level number
	m_numEnemies = enemies;	//number of enemies to generate on this level

	//calculating the offset using which all objects will be created
	if (m_level == 1)
		m_levelOffset = Vector3(0.0f, 0.0f, 0.0f); //if it's level 1, its 1st tile is at 0,0,0
	else
		//if its level 2 to be generated then we need oofset that is full maze size with adjustments to fit a ramp properly
		m_levelOffset = Vector3((m_width*m_tileSize + 4.0f), m_tileSize, (m_height*m_tileSize - 6.0f));

	m_maze = new Tile[m_width*m_height];	//create array of tiles
	RegenerateMaze();	//generate maze
}

MazeGenerator::~MazeGenerator()
{
	//delete tile array
	delete m_maze; 
	m_maze = nullptr;
	//delete wall objects
	delete m_wall;
	m_wall = nullptr;
	//and floor object
	delete m_floorTile;
	m_floorTile = nullptr;

	//now I need to delete enemies
	for (unsigned int i = 0; i < m_listEnemies.size(); i++)
	{
		m_listEnemies[i]->Delete();
	}
	//now erase the list of pointers
	m_listEnemies.clear();
	//now I need to delete pickUps
	for (unsigned int i = 0; i < m_listPickUps.size(); i++)
	{
		delete m_listPickUps[i];
	}
	//now erase the list of pointers
	m_listPickUps.clear();

}

//called form constructor
void MazeGenerator::RegenerateMaze()
{
	//fill the maze with empty tiles
	for (int i = 0; i < m_width*m_height; i++)
	{
		m_maze[i].isSolid = true;	//there is no wall yet
		m_maze[i].isVisited = false;	//it's not yet isVisited
	}
	//random seed using time, so every time you launch the application you get a new maze
	srand(time(NULL));
	// pick a random starting position
	int x = (rand() % (m_width - 2)) | 1;	//position has to be odd number,
	int y = (rand() % (m_height - 2)) | 1;	//because on even numbers might be wall
	IntVec2 tile(x, y); //random starting tile

	m_mazeStart = tile; //temporary assigned to start tile value 
	m_mazeEnd = tile;
	// call the maze generator
	RecurseMazeGenerate(tile);
	FindStartEnd(); //calculates start and end tiles, since maze is generated
	
}

//is valid if inside maze bounds and not visited
bool MazeGenerator::CheckTileValid(IntVec2 &tile)
{
	if ((tile.x < 0) || (tile.x >= m_width) || (tile.y < 0) || (tile.y >= m_height))
		return false;
	return !IsVisited(tile);
}
//maze generetion function
void MazeGenerator::RecurseMazeGenerate(IntVec2 &v)
{
	// locate a random neighbouring cell that hasn't been visited
	vector<IntVec2> possibles; // 4 possible directions
	possibles.push_back(IntVec2(0, -1));
	possibles.push_back(IntVec2(1, 0));
	possibles.push_back(IntVec2(0, 1));
	possibles.push_back(IntVec2(-1, 0));

	// randomise the list using a STD function
	random_shuffle(possibles.begin(), possibles.end());

	// iterate the list going in that direction
	for (unsigned int i = 0; i < possibles.size(); i++)
	{
		// check the direction is valid
		if (CheckTileValid(v + 2 * possibles[i]))
		{
			// mark squares in this direction
			IntVec2 newPos = v + possibles[i];
			m_maze[newPos.y*m_width + newPos.x].isSolid = false;
			m_maze[newPos.y*m_width + newPos.x].isVisited = true;
			newPos += possibles[i];
			m_maze[newPos.y*m_width + newPos.x].isSolid = false;
			m_maze[newPos.y*m_height + newPos.x].isVisited = true;
			// continue recursing
			RecurseMazeGenerate(newPos);
		}
	}
}

//route finding functions
#define MAX_WEIGHT 0x7fffffff // largest possible value for a signed 32 bit integer
//initialise route finding 
void MazeGenerator::InitDijkstra(IntVec2 p1, IntVec2 p2)
{
	// set the weight of the start tile to 0, and the rest to infinity, marking all tiles as unvisited
	for (int i = 0; i < m_width*m_height; i++)
	{
		m_maze[i].weight = MAX_WEIGHT;
		m_maze[i].isVisited = false;
		m_maze[i].isPath = false;
	}
	GetTile(p1).weight = 0;
	GetTile(p1).backTrack = p1;
	// store the search start and search end points
	m_searchStart = p1;
	m_searchEnd = p2;

	return;
}

bool MazeGenerator::StepDijkstra()
{
	IntVec2 candidate(-1, -1);
	int minWeight = MAX_WEIGHT;
	// the first step in each step is to find the lowest weight unvisited tile and go from there
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			IntVec2 tile(x, y);
			if ((!IsSolid(tile)) && (!IsVisited(tile)))
			{ // not solid and not visited
				int weight = GetTile(tile).weight;
				if (weight < minWeight)
				{ // set candidate & replace minimun weight
					candidate = tile;
					minWeight = weight;
				}
			}
		}
	}
	// did we find a candidate - if not there's no path to the destination
	if (minWeight == MAX_WEIGHT)
		return false;
	// mark the candidate as visited
	GetTile(candidate).isVisited = true;
	Tile &t = GetTile(candidate);

	// get it's weight + 1 - which would be the weight of the next square
	int weight = GetTile(candidate).weight + 1;

	// look at all the directions from this tile and if they're valid and unvisited check if the current path is shorter than the lowest found so far
	IntVec2 newPos = candidate + IntVec2(0, -1); // go north
	if ((!IsSolid(newPos)) && (!IsVisited(newPos)))
	{
		// if the weight of this tile is greater than our weight+1 set it to our weight+1 and set the backtrack direction
		if (GetTile(newPos).weight > weight)
		{
			GetTile(newPos).weight = weight;
			GetTile(newPos).backTrack = candidate;
		}
	}

	newPos = candidate + IntVec2(1, 0); // go east
	if ((!IsSolid(newPos)) && (!IsVisited(newPos)))
	{
		// if the weight of this tile is greater than our weight+1 set it to our weight+1 and set the backtrack direction
		if (GetTile(newPos).weight > weight)
		{
			GetTile(newPos).weight = weight;
			GetTile(newPos).backTrack = candidate;
		}
	}

	newPos = candidate + IntVec2(0, 1); // go south
	if ((!IsSolid(newPos)) && (!IsVisited(newPos)))
	{
		// if the weight of this tile is greater than our weight+1 set it to our weight+1 and set the backtrack direction
		if (GetTile(newPos).weight > weight)
		{
			GetTile(newPos).weight = weight;
			GetTile(newPos).backTrack = candidate;
		}
	}

	newPos = candidate + IntVec2(-1, 0); // go west
	if ((!IsSolid(newPos)) && (!IsVisited(newPos)))
	{
		// if the weight of this tile is greater than our weight+1 set it to our weight+1 and set the backtrack direction
		if (GetTile(newPos).weight > weight)
		{
			GetTile(newPos).weight = weight;
			GetTile(newPos).backTrack = candidate;
		}
	}

	return true;
}

bool MazeGenerator::IsDijkstraFinished(vector<IntVec2> &ret)
{
	ret.clear();

	// if the searchEnd tile is marked as visited we're done and can retrieve the path
	if (!IsVisited(m_searchEnd))
		return false;

	// the shortest path has been found
	// reset the visited tiles so we can mark only the path
	ResetVisited();
	GetTile(m_searchEnd).isVisited = true; // remember we finished
	// backtrack till we reach the start storing each point into the ret vector
	IntVec2 pos = m_searchEnd;
	ret.push_back(pos); // always insert the end point
	GetTile(pos).isPath = true;
	while (pos != m_searchStart)
	{
		pos = GetTile(pos).backTrack; // step back
		ret.push_back(pos); // add this tile
		GetTile(pos).isPath = true;
	}
	// the list we created is backwards, so we reverse it
	reverse(ret.begin(), ret.end());
	return true;
}
//function resets all tiles as unvisited
void MazeGenerator::ResetVisited()
{
	for (int i = 0; i < m_width*m_height; i++)
	{
		m_maze[i].isVisited = false;
	}
}
//main dijkstra loop function
void MazeGenerator::RunDijkstra(IntVec2 p1, IntVec2 p2, vector<IntVec2> &ret)
{
	InitDijkstra(p1, p2);
	while (!IsDijkstraFinished(ret))
		StepDijkstra();
}


//initialise function creates all the maze objects in their positions
void MazeGenerator::Initialise()
{
	//creating wall & tile objects
	m_wall = new Wall(m_engine, m_level);
	m_floorTile = new FloorTile(m_engine, m_level);
	//not adding this objects to the engine yet because AddRenderableObjects function calls Initialise but Instance buffer
	//in Initialise function cant be created without the list of positions
	
	// start by rendering the walls & tiles
	//going throught all the tiles
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			IntVec2 currentTile(x, y); //current tile
			float posX, posY, posZ;

			posX = (float)(currentTile.x) * m_tileSize + m_levelOffset.x; //calculating the tile positon in world units
			posY = 0.0f + m_levelOffset.y;
			posZ = (float)(currentTile.y) * m_tileSize + m_levelOffset.z;

			//enter & exit tiles are precaclculated after maze is generated
			//if tile isSolid & it's not enter or exit tile i can render a wall there
			if ((IsSolid(currentTile)) && (currentTile != m_enterTile) && (currentTile != m_exitTile))
			{
				//adding wall instance
				Vector3 pos = Vector3(posX, posY, posZ);
				m_wall->AddInstance(pos);
				//adding wall position to a list of walls for collision checking
				m_engine->AddWallCollider(pos, m_level);
			}
			else  
			{
				//if it's not a wall, floor has to be covered
				//calculate position
				//Y is lower, because I don't want tiles to be rendered in the middle of the wall, but at the bootom of it
				Vector3 pos = Vector3(posX, posY - m_tileSize/2, posZ); 
				//adding another floor instance
				m_floorTile->AddInstance(pos);
			}
		}
	}
	//now I can add them to the renderable list and initialise ia the lists of instances are finished 
	m_engine->AddRenderableObject(m_wall, true, false); //wall is asting shadows, but is not transparent
	m_engine->AddRenderableObject(m_floorTile, false, false);	//floor tile is not castig shadows
	//call the function to set the enemies and pick ups in the level
	GeneratePickUps();
	SetEnemies();
}

//function that randomly allocates pickups around the maze
void MazeGenerator::GeneratePickUps()
{
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			IntVec2 currentTile(x, y);
			if (!IsSolid(currentTile))
			{
				int random = rand() % 100; //random so pickups are not spawned on every tile
				if (random <= 10)	//1 of 10 tiles is gonna have pickup spawned ( 10% spawn rate)
				{
					//calculating position
					Vector3 pos = Vector3(float(x), 0.0f, float(y));
					pos *= m_tileSize; //multiply by size of any tile
					pos += m_levelOffset; //add the level offset to get the right position relevant to the level number
					Collectible* pickUp = new Collectible(m_engine); //create new pickup
					m_engine->AddRenderableObject(pickUp, true, false); //add it to objects lst for rendering (true - casts shadown, false - not transparent)
					m_engine->AddPickUpCollider(pickUp); //adding pick to collider list 
					AddToPickUpList(pickUp); //this is separate pick up member list
					//set the position and scale
					pickUp->SetPosition(pos); 
					pickUp->SetScale(0.2f);

				}
			}
		}
	}
}
//function that sets enemies
void MazeGenerator::SetEnemies()
{
	if (m_level == 1) //if it's level 1 spawn Squasher
	{
		for (int i = 0; i < m_numEnemies; i++)
		{
			//enemy needs a reference to the maze it's created to have acces to route finding
			Squasher* enemy = new Squasher(m_engine, this);
			enemy->SetPosition(0.0f, 0.0f, 0.0f); //position is recalculated using route findng algorithm
			enemy->SetScale(1.0f);
			m_engine->AddRenderableObject(enemy, true, false);	//add as a renderable object
			AddToEnemyList(enemy); //adding enemy to the member list of enemies
			m_engine->AddEnemyCollider(enemy); //add to the list of enemy colliders
		}
		
	}
	else //if its level2 spawn fire demon
	{
		for (int i = 0; i < m_numEnemies; i++)
		{
			
			FireDemon* enemy = new FireDemon(m_engine, this);
			enemy->SetPosition(0.0f, 0.0f, 0.0f); //position is recalculated using route findng algorithm
			enemy->SetScale(0.7f);
			m_engine->AddRenderableObject(enemy, true, false); //add as a renderable object
			AddToEnemyList(enemy); //adding enemy to the list
			//fire demon is not added as a collider , instead it's bullets are
		}
	}
	
}
//Add and remove enemies from the member list
void MazeGenerator::AddToEnemyList(IEnemy* pObject)
{
	m_listEnemies.push_back(pObject);
}
void MazeGenerator::RemoveFromEnemyList(IEnemy* pObject)
{
	//iterate through list to find the right objects
	for (unsigned int i = 0; i < m_listEnemies.size(); i++)
	{
		if (m_listEnemies[i] == pObject)
		{
			//erase the object
			m_listEnemies.erase(m_listEnemies.begin() + i);
			break;
		}
	}
}
//add and remove pickUps from the member list
void MazeGenerator::AddToPickUpList(Collectible* pObject)
{
	m_listPickUps.push_back(pObject);
}
void MazeGenerator::RemoveFromPickUpList(Collectible* pObject)
{
	//iterate through list to find the right objects
	for (unsigned int i = 0; i < m_listPickUps.size(); i++)
	{
		if (m_listPickUps[i] == pObject)
		{
			//erase object
			m_listPickUps.erase(m_listPickUps.begin() + i);
			break;
		}
	}
}
//destroying pick up ( when its collected by player)
void MazeGenerator::DeletePickUp(Collectible* pObject)
{
	//1st have to delete it from renderable list
	m_engine->RemoveRenderableObject(pObject, true, false);
	//removing from collider list
	m_engine->RemovePickUpCollider(pObject);
	//and removing from member pickup list
	RemoveFromPickUpList(pObject);
	//and finally call destructor on the object
	delete pObject;

	return;
}

//maze updates objects inside of it 
void MazeGenerator::Update(float dTime)
{
	//first need to update pickups
	//iterate through pick up list
	for (unsigned int i = 0; i < m_listPickUps.size(); i++)
	{
		m_listPickUps[i]->Update(dTime); //update pickups
		//check if pick up is active (when collided with player, pickup is set a inactive, here I clean them up)
		if (!m_listPickUps[i]->IsActive()) //if pickUp is not active
			DeletePickUp(m_listPickUps[i]); //delete that pickup
	}
	//going through the enemy list
	for (unsigned int i = 0; i < m_listEnemies.size(); i++)
	{
		m_listEnemies[i]->Update(dTime); //update every enemy
	}

}

void MazeGenerator::FindStartEnd()
{
	//logically start tile would be smallest pos and not wall, opposite applies to end tile
	//going through all tiles
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			IntVec2 currentTile(x, y); //curent tile
			if (!IsSolid(currentTile)) //if tile is not wall
			{
				//now recursing through all the not wall tiles and obtaining the smalles x, y value & the biggest x nd y values
				//this algorithm could be very simplified as the start & end will always be the same position in my maze layout
				if (currentTile.x < m_mazeStart.x)
				{
					m_mazeStart.x = currentTile.x;
				}
				else if (currentTile.x > m_mazeEnd.x)
				{
					m_mazeEnd.x = currentTile.x;
				}

				if (currentTile.y < m_mazeStart.y)
				{
					m_mazeStart.y = currentTile.y;
				}
				else if (currentTile.y > m_mazeEnd.y)
				{
					m_mazeEnd.y = currentTile.y;
				}

			}

		}
	}
	FindRampTiles(); //find the tiles from which you enter and exit the ramp

}
//this function finds the tiles from which you enter and exit the ramp
void MazeGenerator::FindRampTiles()
{
	if (m_level == 1)
	{
		m_enterTile = IntVec2(-200, -200); //on level 1 theres no enter tile as we start inside the maze
		m_exitTile = m_mazeEnd + IntVec2(1, 0); //add 1 on X axis , as you exit to the right side of the end tile
	}
	else //if its level2
	{
		m_exitTile = IntVec2(-200, -200); //we dont need exit tile on level2, as game is finished there
		m_enterTile = m_mazeStart - IntVec2(1, 0); //deduct 1 on X axis as entrance is on the left from first tile
	}
}


//getters
//retrieves the position of the final tile
Vector3 MazeGenerator::GetEndPosition()
{
	Vector3 ret;
	//mazeEnd is IntVec2 so I have to transfer it to the world position; multiplying by size of tile and adding offset of the current level
	float x = m_mazeEnd.x * m_tileSize + m_levelOffset.x; 
	float z = m_mazeEnd.y * m_tileSize + m_levelOffset.z;
	float y = 0 + m_levelOffset.y;
	ret = Vector3(x, y, z); 
	return ret;
}
//retrieves the first tile position to sawn player on
Vector3 MazeGenerator::GetStartPosition()
{
	Vector3 ret;
	//mazeStart is IntVec2 so I have to transfer it to the world position; multiplying by size of tile and adding offset of the current level
	float x = m_mazeStart.x * m_tileSize + m_levelOffset.x;
	float z = m_mazeStart.y * m_tileSize + m_levelOffset.z;
	float y = 0 + m_levelOffset.y;
	ret = Vector3(x, y, z);
	return ret;
}


