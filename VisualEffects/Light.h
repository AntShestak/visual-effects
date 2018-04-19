#pragma once

//class that imitates the light source
class Light
{
public:
	//overaload for operators new & delete for allocation purposes
	void* operator new(size_t i)
	{
		return _aligned_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_aligned_free(p);
	}
	//constructor & destructor
	Light();
	~Light();

	void Initialise(); //initialises the light properties

	void GenerateViewMatrix(); //function generates light's view matrix
	void GenerateProjectionMatrix(float nearZ, float farZ); //function generates projection matrix, takes clip plane distances
	
	//accessors
	Vector3 GetPosition(); //returns lights position
	Vector3 GetLookAt();	//returns the look at position
	XMMATRIX GetViewMatrix(); //returns view matrix
	XMMATRIX GetProjectionMatrix();	//returns projection matrix
	//setters
	void SetPosition(float x, float y, float z); //sets light position
	void SetLookAt(float x, float y, float z); //sets position to look at

private:
	__declspec(align(16))
	XMMATRIX m_viewMatrix; //view matrix
	XMMATRIX m_projectionMatrix; //projection matrix

	Vector3 m_position; //light's position
	Vector3 m_lookAt;	//look at point
};