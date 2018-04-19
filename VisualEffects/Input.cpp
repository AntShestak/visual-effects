#include "stdafx.h"
#include "Input.h"

Input::Input() //construcotr
{
	m_cameraMode = 3; // by default
}

Input::~Input() //destructor
{
	//nothing to do
}

Vector3 Input::GetMovement() //returns movement vector
{
	m_movement = Vector3(0, 0, 0);

	//using Win32 function to get the keyboard status
	BYTE keys[256];
	if (!GetKeyboardState(keys)) //if no keyboard pressed return 0 movement
		return Vector3(0,0,0);

	/*
		NOTE: "If" statement is used instead of "else if" so it's possible to get input from multiple keys
	*/
	if (keys[0x57] & 0x80) // if W is pressed
	{
		m_movement.z += 1.0f; //move Z axis, as it's forward for the camera
	}
	if (keys[0x53] & 0x80) // if S is pressed
	{
		m_movement.z -= 1.0f; //move -Z axis
	}
	if (keys[0x41] & 0x80) //if A key pressed
	{
		m_movement.x -= 1.0f; //move left
	}
	if (keys[0x44] & 0x80) // D key
	{
		m_movement.x += 1.0f; //move right
	}
	//return calculated movement
	return m_movement;
}

int Input::GetCameraMode() //returns int variable that describes camera mode to be used
{
	//using Win32 function to get the keyboard status
	BYTE keys[256];
	if (!GetKeyboardState(keys)) 
		return m_cameraMode; //returns not updated camera mode if no keyboard pressed
	
	if (keys[0x49] & 0x80) //if "I" key is pressed
	{
		//1 is 1st person mode
		m_cameraMode = 1;
	}
	else if (keys[0x4f] & 0x80) //if "O" is pressed
	{
		// 3 is third person view
		m_cameraMode = 3;
	}

	return m_cameraMode;
}

int Input::GetZoom() //return positive or negative 1 depending on input if zoom in or zoom out is requsted
{
	//using Win32 function to get the keyboard status
	BYTE keys[256];
	if (!GetKeyboardState(keys))
		return 0; //if nothing pressed returns 0;

	if (keys[0x4B] & 0x80) //if K key is pressed
	{
		//if K is pressed zoom out
		return 1;
	}
	else if (keys[0x4C] & 0x80) //if L key is pressed
	{
		//when L is pressed zoom in
		return -1;
	}
	else return 0;
}