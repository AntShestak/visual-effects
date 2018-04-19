#pragma once

// A simple interface to a renderable object
// inherit from this, implement it and then pass it to the DirectXWrapper

struct Vertex
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
};

struct UVVertex
{
	Vector3 Pos;
	Vector2 UV;
};

struct InstanceData
{
	Vector3 position;
};

class DirectXWrapper; // forward declaration

class IRenderable
{
public:
	virtual void Initialise(DirectXWrapper *wrapper) = 0; // called once when added to the wrapper
	virtual void Render(DirectXWrapper *wrapper,int pass) = 0; // called every frame
	virtual Vector3 GetPosition() = 0; //returns position vector
};