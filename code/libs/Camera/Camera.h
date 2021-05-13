#pragma once
#include <string>
#include "Vector3.h"
#include "Vector2F.h"
#include "Matrix4.h"
class Camera
{
public:
	Camera(const Vector3& initPos = Vector3(0,0,0), int windowWidth = 1024, int windowHeight = 768, double newNearPlane = 0.1, double newFarPlane = 2000.0, double newFov = 45.0);
	~Camera();
	void Update(double deltaTime);

	void CalculateViewMatrix();

	void UpdateProjection();

	void ComputeVectors();

	void UpdatePosition(double deltaTime);

	void UpdateOrientation(double mouseX, double mouseY);

	void UpdateSize(int width, int height);

	Vector3 ConvertMousePosToWorldDir(double mousePosX, double mousePosY);

	Vector3 GetInitPos();
	Vector3 GetPosition();
	Vector3 GetPosition2();

	void SetPosition(Vector3& pos);
	void SetTarget(Vector3& target);

	bool holdingForward;
	bool holdingBackward;
	bool holdingRight;
	bool holdingLeft;
	bool holdingUp;
	bool holdingDown;

	Matrix4 ProjectionMatrix;
	Matrix4 ProjectionInverseMatrix;
	Matrix4 ViewMatrix;
	Matrix4 ViewInverseMatrix;
	union
	{
		struct { float windowWidth, windowHeight; };
		Vector2F screenSize;
	};
	
	double windowMidX;
	double windowMidY;

	float fov;
	float near;
	float far;
	Vector3 target;
	Vector3 up;
	Vector3 right;
	Vector3 forward;
	float speed;
	std::string name;
private:
	Vector3 position;
	double verticalAngle;
	double horizontalAngle;
	double mouseSpeed;
};