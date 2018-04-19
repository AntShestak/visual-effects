#pragma once

//interface for enemy objects so I can group them under once class
class IEnemy
{
public:
	virtual void Update(float) = 0; //function to update object
	virtual void Delete() = 0; //function that destroys an object
};