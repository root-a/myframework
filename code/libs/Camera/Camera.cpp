#include "cmath"
#include "MyMathLib.h"
#include "Camera.h"

using namespace mwm;

Camera::Camera(const mwm::Vector3& initPos, int windowW, int windowH)
{
	// Initial horizontal angle : toward -Z
	horizontalAngle = 3.14;
	// Initial vertical angle : none
	verticalAngle = 0.0;
	// Initial Field of View
	fov = 45.0;
	near = 0.1;
	far = 100;

	speed = 100.0;
	mouseSpeed = 0.002;

	holdingForward = false;
	holdingBackward = false;
	holdingRight = false;
	holdingLeft = false;
	holdingUp = false;
	holdingDown = false;

	position = initPos;

	UpdateSize(windowW, windowH);
	UpdateOrientation(windowMidX, windowMidY + 100);
}

Camera::~Camera()
{
}

Vector3 Camera::getDirection()
{
	return direction;
}

Vector3 Camera::getUp()
{
	return up;
}

Vector3 Camera::GetInitPos()
{
    return position;
}

Vector3 Camera::GetPosition() //no scaling of view
{
	Vector3 pos = ViewMatrix.getPosition();
	Matrix3 rotMat = ~(ViewMatrix.ConvertToMatrix3()); //need to transpose for row major

	Vector3 retVec = rotMat * (-1 * pos);
	return retVec;
}

Vector3 Camera::GetPosition2() //if scaled view
{
	Matrix4 viewModel = ViewMatrix.inverse();
	Vector3 cameraPos = viewModel.getPosition();
	return cameraPos;
}

void Camera::Update(double deltaTime){

	UpdatePosition(deltaTime);

	CalculateViewMatrix();
}

void Camera::UpdateOrientation(double mouseX, double mouseY)
{
	// Compute new orientation

	horizontalAngle += mouseSpeed * (windowMidX - mouseX);
	verticalAngle += mouseSpeed * (windowMidY - mouseY);

	//if monitoring camera
	/*
	if (cameraType == 3)
	{
		verticalAngle = 0.0f;
		horizontalAngle = 3.14f;
	}
	*/

	ComputeVectors();
}

void Camera::UpdatePosition(double deltaTime)
{
	// Move forward
	if (holdingForward){
		position = position + (direction * deltaTime * speed);
	}
	// Move backward
	if (holdingBackward){
		position = position - (direction * deltaTime * speed);
	}
	// Strafe right
	if (holdingRight){
		position = position + (right * deltaTime * speed);
	}
	// Strafe left
	if (holdingLeft){
		position = position - (right * deltaTime * speed);
	}
	if (holdingUp){
		position = position + (up * deltaTime * speed);
	}
	if (holdingDown){
		position = position - (up * deltaTime * speed);
	}
}

void Camera::UpdateSize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	windowMidX = windowWidth / 2.0;
	windowMidY = windowHeight / 2.0;
	UpdateProjection();
}

void Camera::SetFoV(double newFov)
{
	fov = newFov;
	UpdateProjection();
}

void Camera::SetNearPlane(double newNearPlane)
{
	near = newNearPlane;
	UpdateProjection();
}

void Camera::SetFarPlane(double newFarPlane)
{
	far = newFarPlane;
	UpdateProjection();
}

void Camera::ComputeVectors()
{
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = Vector3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	// Right vector
	right = Vector3(
		sin(horizontalAngle - 3.14 / 2.0),
		0.0,
		cos(horizontalAngle - 3.14 / 2.0)
		);

	// Up vector
	up = right.crossProd(direction);
}

void Camera::CalculateViewMatrix()
{
	// Camera matrix
	//if (cameraType == 1 || cameraType == 2)
	//{
		ViewMatrix = Matrix4::lookAt(
			position,           // Camera is here
			position + direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
			);
		/*if (cameraType == 2)
		{
			ViewMatrix = ViewMatrix*Matrix4::translate(0, 0, -10);// we move the camera out after creating the look at (rotation matrix)
		}
	}
	if (cameraType == 3)
	{
		ViewMatrix = Matrix4::lookAt(
			Vector3(0.f, 0.f, 6.f),           // Camera is here
			initialPosition + direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
			);
	}*/
}

void Camera::UpdateProjection()
{
	ProjectionMatrix = Matrix4::OpenGLPersp(fov, (double)windowWidth/(double)windowHeight, near, far);
}

void Camera::SetPosition(mwm::Vector3& pos)
{
	//proper way to set position would be to get the position from matrix via inverse
	//then new position - camera pos -> vector with direction to new position 
	//then we translate the initial position with that vector -> initialPosition += new vector with dir
	//this would be required if one did change the camera matrix directly rather than changing the position variable 
	position = pos;
}

mwm::Vector3 Camera::getRight()
{
	return right;
}


