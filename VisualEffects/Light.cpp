#include "stdafx.h"
#include "Light.h"

Light::Light()
{
	//nothing to do in the constructor
}

Light::~Light()
{
	//nothing to delete in the destructor
}

void Light::Initialise()
{
	SetLookAt(17.0f, 0.0f, 17.0f); //this positon is between 2 mazes
	SetPosition(-10.0f, 50.0f, -15.0f); //this is position of the light source
	//both create the angle that won't change during runtime

	GenerateProjectionMatrix(40.0f, 140.0f); //calculate the prjection matrix with 40 and 140 being near & far clip plane properties respectively
	GenerateViewMatrix(); //generte view matrix
}

void Light::SetPosition(float x, float y, float z) //sets light position
{
	m_position = Vector3(x, y, z);
}

void Light::SetLookAt(float x, float y, float z) //sets look at position
{
	m_lookAt = Vector3(x, y, z);
}

Vector3 Light::GetPosition() //return current position
{
	return m_position;
}

Vector3 Light::GetLookAt() //returns look at position
{
	return m_lookAt;
}

void Light::GenerateViewMatrix() //generates view matrix for light source
{
	//need to transform my Vector3's to XMVECTOR
	XMVECTOR lightPos = XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f);
	XMVECTOR lightLookAt = XMVectorSet(m_lookAt.x, m_lookAt.y, m_lookAt.z, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //vectng upor directi
	//generate matrix
	m_viewMatrix = XMMatrixLookAtLH(lightPos, lightLookAt, Up);

	return;
}

void Light::GenerateProjectionMatrix(float screenNear, float screenDepth) //generates projection matrix for the light source
{
	float size = 50.0f; //size of view square
	m_projectionMatrix = XMMatrixOrthographicLH(size, size, screenNear, screenDepth); //generate matrix using near & far clip planes

	return;
}

XMMATRIX Light::GetViewMatrix() //returs current viewMatrix
{
	return m_viewMatrix;
}

XMMATRIX Light::GetProjectionMatrix() //returns current projection matrix
{
	return m_projectionMatrix;
}
