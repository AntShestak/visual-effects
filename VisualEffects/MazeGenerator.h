#pragma once

#include "GraphicsEngine.h"
//objects used in maze
#include "Wall.h"
#include "FloorTile.h"
#include "Collectible.h" 
#include "IEnemy.h"		//to create ist of enemies using iEnemy interface

//integer 2d vector class
class IntVec2
{
public:
	int x, y;
	IntVec2() { x = 0; y = 0; }
	IntVec2(int xVal, int yVal) { x = xVal; y = yVal; }
	//operator overaloads
	bool operator == (const IntVec2& V) const { return (V.x == x) && (V.y == y); }
	bool operator != (const IntVec2& V) const { return (V.x != x) || (V.y != y); }
	IntVec2& operator= (const IntVec2& V) { x = V.x; y = V.y; return *this; }
	IntVec2& operator+= (const IntVec2& V) { x += V.x; y += V.y; return *this; }
	IntVec2& operator-= (const IntVec2& V) { x -= V.x; y -= V.y; return *this; }
	IntVec2& operator*= (int S) { x *= S; y *= S; return *this; }
	IntVec2& operator/= (int S) { x /= S; y /= S; return *this; }
};

// Binary operators
IntVec2 operator+ (const IntVec2& V1, const IntVec2& V2);
IntVec2 operator- (const IntVec2& V1, const IntVec2& V2);
IntVec2 operator* (const IntVec2& V, int S);
IntVec2 operator* (int S, const IntVec2& V);


// structure with tile properties
struct Tile
{
	IntVec2 backTrack; // used to store the path 
	IntVec2 forwardTrack; // used by Wall Follower to store the forward path
	int weight; //tile weight for dijkstra
	bool isSolid : 1; // this tile is a solid wall
	bool isVisited : 1;
	bool isPath : 1; // tile is marked as path 
};

class MazeGenerator
{
public:
	//constructor. W -width, H - height. Size = dimension of a square tile. Stage - is the level number. Enemies - number of enemies in this level.
	MazeGenerator(GraphicsEngine* ge, int w, int h, int size, int stage, int enemies);
	~MazeGenerator();

	void Initialise(); //called once
	void Update(float dTime); //called every frame

	// used by enemy, to find a route
	void RunDijkstra(IntVec2 p1, IntVec2 p2, vector<IntVec2> &ret);
	
	//objects control
	void GeneratePickUps();	//creates pickUps
	void SetEnemies();  //creates enemies
	void AddToPickUpList(Collectible* pObject); //add to separate list for pickups
	void RemoveFromPickUpList(Collectible* pObject); //remove from list of pickups
	void AddToEnemyList(IEnemy* pObject);		//add to enemy list, using IEnemy interface
	void RemoveFromEnemyList(IEnemy* pObject);	//remove from enemy list
	void DeletePickUp(Collectible* pObject); // deletes a pick up
	
	//Getters
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	bool IsSolid(IntVec2 pos) { return m_maze[pos.y*m_width + pos.x].isSolid; }
	Tile &GetTile(IntVec2 pos) { return m_maze[pos.y*m_width + pos.x]; }
	Vector3 GetOffset() { return m_levelOffset; }	//returns the offset used to render mazes on different positions
	//IntVec2 GetStartPosition() { return m_mazeStart; }
	Vector3 GetEndPosition();
	Vector3 GetStartPosition();
	IntVec2 GetRampTile() { return m_exitTile; }
	float GetTileSize(){ return m_tileSize; }

private:
	//private functions
	MazeGenerator(); // hide the default constructor
	//maze generation functions
	void RegenerateMaze();
	bool CheckTileValid(IntVec2 &v);
	void RecurseMazeGenerate(IntVec2 &v);
	bool IsVisited(IntVec2 pos) { return m_maze[pos.y*m_width + pos.x].isVisited; }
	void ResetVisited();
	void FindStartEnd(); //function to calculate start & end  tiles
	void FindRampTiles();
	//route finding
	void InitDijkstra(IntVec2 p1, IntVec2 p2);
	bool StepDijkstra(); // returns false if no path
	bool IsDijkstraFinished(vector<IntVec2> &ret); // returns true if we reached the destination

	GraphicsEngine* m_engine = nullptr; //reference to engine
	Wall* m_wall = nullptr;				//objects used in maze
	FloorTile* m_floorTile = nullptr;

	//lists of objects
	vector<Collectible*> m_listPickUps; //list of pickups
	vector<IEnemy*> m_listEnemies; // list of objects that require update every frame

	//member variables
	Tile* m_maze = nullptr;		//pointer to an array of tiles
	int m_level;				//level number
	int m_width, m_height;		//level dimension ( not accounting on tile sizes)
	int m_tileSize;				//tilesize
	Vector3 m_levelOffset; //this offset is used to locate position of maze depending on level
	int m_numEnemies; //enemy count assigned is constructor
	//djikstra
	IntVec2 m_searchStart, m_searchEnd; // current search start and end points
	//other variables
	IntVec2	m_mazeStart, m_mazeEnd; //maze start and maze emd tiles
	IntVec2 m_enterTile, m_exitTile; //this tiles are used in ram position calculations
	

};