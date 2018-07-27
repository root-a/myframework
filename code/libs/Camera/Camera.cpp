#include "cmath"
#include "MyMathLib.h"
#include "Camera.h"
#include <stdio.h>
using namespace mwm;

Camera::Camera(const mwm::Vector3& initPos, int windowW, int windowH, double newNearPlane, double newFarPlane, double newFov)
{
	// Initial horizontal angle : toward -Z
	horizontalAngle = 3.14;
	// Initial vertical angle : none
	verticalAngle = 0.0;
	// Initial Field of View
	fov = newFov;
	near = newNearPlane;
	far = newFarPlane;

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

	ComputeVectors();

	CalculateViewMatrix();

	UpdateProjection();
}

void Camera::UpdateOrientation(double mouseX, double mouseY)
{
	// Compute new orientation

	horizontalAngle += mouseSpeed * (windowMidX - mouseX);
	verticalAngle += mouseSpeed * (windowMidY - mouseY);
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
}

void Camera::SetFoV(double newFov)
{
	fov = newFov;
}

void Camera::SetNearPlane(double newNearPlane)
{
	near = newNearPlane;
}

void Camera::SetFarPlane(double newFarPlane)
{
	far = newFarPlane;
}

void Camera::SetFarNearFov(double newFov, double newNearPlane, double newFarPlane)
{
	fov = newFov;
	near = newNearPlane;
	far = newFarPlane;
}

Vector3 Camera::ConvertMousePosToWorldDir(double mousePosX, double mousePosY)
{
	Vector4 mouse_p0s = Vector4();
	mouse_p0s.x = (mousePosX / (double)windowWidth)*2.0 - 1.0;
	mouse_p0s.y = (((double)windowHeight - mousePosY) / (double)windowHeight)*2.0 - 1.0;
	mouse_p0s.z = -1.0;
	mouse_p0s.w = 1.0;

	Vector4 my_mouse_in_world_space = ProjectionMatrix.inverse() * mouse_p0s;
	my_mouse_in_world_space.z = -1;
	my_mouse_in_world_space.w = 0;
	my_mouse_in_world_space = ViewMatrix.inverse() * my_mouse_in_world_space;

	return my_mouse_in_world_space.get_xyz().vectNormalize();
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

	ViewMatrix = Matrix4::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
		);
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


