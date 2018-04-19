#pragma once

//an interface to a collidable object
class ICollidable
{
public:
	virtual void Destroy() = 0; //function that will destroy the object that has collided if needed
	virtual float GetScale() = 0; //returns scale of an object
	virtual Vector3 GetPosition() = 0; //return vector of position
};