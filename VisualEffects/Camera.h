#pragma once

#include "DirectXWrapper.h"

//class that control camera behaviour
class Camera
{
public:
	//overload for new & delete operators for alignment reasons
	void* operator new(size_t i)
	{
		return _aligned_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_aligned_free(p);
	}

	//constructor & destructor
	Camera();
	~Camera();

	void Initialise(DirectXWrapper* dx, HWND handle); //initialises camera, passing directX reference and window handle
	//functions to zoom camera in & out
	void ZoomIn();
	void ZoomOut();
	//update functions 
	void SetFirstPerson(float x, float y, float z); //sets camera in 1st person mode and updates
	void SetThirdPerson(float x, float y ,float z);	//sets camera in 3rd person mode and updates
	//accessors
	XMMATRIX GetRotationY(){ return m_rotationY; } //returns camera rotation around Y axis
	XMMATRIX GetViewMatrix(){ return m_viewMatrix; } //returns viewMatrix of the camera
	XMMATRIX GetProjectionMatrix();					//calculates & returns projection matrix

private:
	
	DirectXWrapper* m_dx = nullptr; //reference to directx
	HWND m_hwnd;	//window handle

	float m_angle; //zoom angle
	
	// aligned camera matrices
	__declspec(align(16))
	XMMATRIX			m_rotationY; // this variable must have same rotation as camera BUT ONLY on Y axis (used for player & particles )
	XMMATRIX			m_rotation; //camera's rotation matrix
	XMMATRIX            m_viewMatrix; //view matrix
	XMMATRIX            m_projMatrix;	//projection matrix

};