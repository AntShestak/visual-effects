#include "stdafx.h"
#include "Camera.h"

#define _USE_MATH_DEFINES //needed for access to PI value
#include <math.h>

//for convertion from screen pixels to rdians we say that 1000 pixels on the screen is one full rotation(360 degrees)
#define MOUSE_SCALING ((float)(2.0*M_PI/1000.0))

Camera::Camera()
{
	//nothing to do in constructor
}

Camera::~Camera()
{
	//npthing to do in destructor
}

void Camera::Initialise(DirectXWrapper* dx, HWND handle)
{
	m_dx = dx;		//reference to directX
	m_hwnd = handle;	//assign handle
	
	m_angle = 90.0f; //default zoom angle is 90 degrees
}


void Camera::ZoomOut()
{
	//when zoom out is called increasing angle for projection calculations
	m_angle += 0.5f;
	if (m_angle > 105.0f)
		m_angle = 105.0f; //clamping max angle

	return;
}

void Camera::ZoomIn()
{
	//when zoom in is called decreasing the angle for projection calculation
	m_angle -= 0.5f;
	if (m_angle < 65.0f) //clamping minimum angle
		m_angle = 65.0f;

	return;
}

void Camera::SetThirdPerson(float x, float y, float z) //called by player providing his location
{
	// get the camera orientation from the mouse position
	POINT p;
	GetCursorPos(&p); // this gets the Windows mouse cursor position in screen coordinates
	//need to scale it to radians using defined value 
	float yaw = (float)p.x * MOUSE_SCALING;
	float pitch = ((float)p.y * MOUSE_SCALING) - (float)M_PI / 2;
	if (pitch > (float)M_PI / 2)
		pitch = (float)M_PI / 2;
	float roll = 0;
	//updating special rotation variable, explained in camera.h
	m_rotationY = XMMatrixRotationY(yaw); //this rotation is not used in camera code
	// calculate rotation matrix
	m_rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	// using 4D vector define stadard axis
	XMVECTOR xAxis = XMLoadFloat3(&XMFLOAT3(1, 0, 0));
	XMVECTOR yAxis = XMLoadFloat3(&XMFLOAT3(0, 1, 0));
	XMVECTOR zAxis = XMLoadFloat3(&XMFLOAT3(0, 0, 1));
	// rotate our standard axes and see where they point now
	xAxis = XMVector3Transform(xAxis, m_rotation);
	yAxis = XMVector3Transform(yAxis, m_rotation);
	zAxis = XMVector3Transform(zAxis, m_rotation);

	XMFLOAT3 pos = XMFLOAT3(x, y, z); //set the variable to players position passed
	XMVECTOR playerPosition = XMLoadFloat3(&pos); //load it into XMVECTOR type

	//calculate camera's position
	XMVECTOR pos4 = playerPosition - zAxis*4 + yAxis*4; // so we are 4 units behind and 4 units higher than player
	//calculate look at position
	XMVECTOR lookAt = playerPosition; // camera if focused on the player
	//calculate view matrix
	m_viewMatrix = XMMatrixLookAtLH(pos4, lookAt, yAxis);

}

void Camera::SetFirstPerson(float x, float y, float z)
{
	// get the camera orientation from the mouse position
	POINT p;
	GetCursorPos(&p); // this gets the Windows mouse cursor position in screen coordinates
	//need to scale it to radians using defined value 
	float yaw = (float)p.x * MOUSE_SCALING;
	float pitch = ((float)p.y * MOUSE_SCALING) - (float)M_PI / 2;
	if (pitch > (float)M_PI / 2)
		pitch = (float)M_PI / 2;
	float roll = 0;

	//updating player's rotation variable, explained in camera.h
	m_rotationY = XMMatrixRotationY(yaw); //not used in camera code
	// calculate rotation matrix
	m_rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	// set the standard axis in 4D vector type
	XMVECTOR xAxis = XMLoadFloat3(&XMFLOAT3(1, 0, 0));
	XMVECTOR yAxis = XMLoadFloat3(&XMFLOAT3(0, 1, 0));
	XMVECTOR zAxis = XMLoadFloat3(&XMFLOAT3(0, 0, 1));
	// rotate standard axes and see where they point 
	xAxis = XMVector3Transform(xAxis, m_rotation);
	yAxis = XMVector3Transform(yAxis, m_rotation);
	zAxis = XMVector3Transform(zAxis, m_rotation);

	XMFLOAT3 pos = XMFLOAT3(x, y, z); //assign players position to this temporary float3
	XMVECTOR playerPosition = XMLoadFloat3(&pos); //load player's position into 4D vector type

	//calculate camera's position
	XMVECTOR pos4 = playerPosition +yAxis * 0.5f; // camera will be slightly above player ( just a 1/2unit makes it look slightly better )
	//calculate look at position
	XMVECTOR lookAt = playerPosition + zAxis; // camera looks 1 unit in front of the player
	//calculate viw matrix
	m_viewMatrix = XMMatrixLookAtLH(pos4, lookAt, yAxis);

	return;
}

XMMATRIX Camera::GetProjectionMatrix()
{
	// get the rectangular size of our window
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// projection matrix - recalculate each frame in case window dimensions changed
	m_projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_angle), width / (FLOAT)height, 0.01f, 100.0f);
	
	return m_projMatrix;
}