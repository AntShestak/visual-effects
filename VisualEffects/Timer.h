#pragma once
//class that takes care of timing
class Timer
{
public:
	//contructor & destructor
	Timer();
	~Timer();

	void Initialize(); //initialises class & starts timer

	float GetTime(); //returns delta time

private:
	//large integer to store last frame time
	LARGE_INTEGER m_tLastFrame;

};