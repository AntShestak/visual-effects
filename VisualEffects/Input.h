#pragma once

//class that takes care of keyboard inputs
class Input
{
public:
	Input();
	~Input();

	Vector3 GetMovement(); //returns movement vector
	int GetCameraMode(); //returns camera mode; 1 for 1st person, 3 - 3rd person
	int GetZoom();	//return positive 1 for zoom out, or negative -1 for zoom in

private:
	int m_cameraMode; //camera mode
	Vector3 m_movement;	 //movement vector
};