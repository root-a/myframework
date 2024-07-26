#include "cmath"
#include "MyMathLib.h"
#include "Camera.h"
#include <stdio.h>


Camera::Camera(const glm::vec3& initPos, int windowW, int windowH, double newNearPlane, double newFarPlane, double newFov)
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

	shadowNearOffset = 40.0;
	shadowDistance = 150.0;

	position = initPos;

	UpdateSize(windowW, windowH);
	UpdateOrientation(windowMidX, windowMidY + 100);
}

Camera::~Camera()
{
}

glm::vec3 Camera::GetInitPos()
{
    return position;
}

glm::vec3 Camera::GetPosition() //no scaling of view
{
	glm::vec3 pos = -MathUtils::GetPosition(ViewMatrix);
	glm::mat3 rotMat = glm::transpose(glm::mat3(ViewMatrix)); //need to transpose for row major
	return rotMat * pos;
}

glm::vec3 Camera::GetPosition2() //if scaled view
{
	return MathUtils::GetPosition(ViewInverseMatrix);
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
	float deltaTimeF = deltaTime;
	// Move forward
	if (holdingForward){
		position = position + (forward * deltaTimeF * speed);
	}
	// Move backward
	if (holdingBackward){
		position = position - (forward * deltaTimeF * speed);
	}
	// Strafe right
	if (holdingRight){
		position = position + (right * deltaTimeF * speed);
	}
	// Strafe left
	if (holdingLeft){
		position = position - (right * deltaTimeF * speed);
	}
	if (holdingUp){
		position = position + (up * deltaTimeF * speed);
	}
	if (holdingDown){
		position = position - (up * deltaTimeF * speed);
	}
}

void Camera::UpdateSize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	windowMidX = windowWidth / 2.0;
	windowMidY = windowHeight / 2.0;
}

glm::vec3 Camera::ConvertMousePosToWorldDir(double mousePosX, double mousePosY)
{
	glm::dvec4 mouse_p0s = glm::dvec4();
	mouse_p0s.x = (mousePosX / (double)windowWidth)*2.0 - 1.0;
	mouse_p0s.y = (((double)windowHeight - mousePosY) / (double)windowHeight)*2.0 - 1.0;
	mouse_p0s.z = -1.0;
	mouse_p0s.w = 1.0;

	glm::dvec4 my_mouse_in_world_space = ProjectionInverseMatrix * mouse_p0s;
	my_mouse_in_world_space.z = -1;
	my_mouse_in_world_space.w = 0;
	my_mouse_in_world_space = ViewInverseMatrix * my_mouse_in_world_space;

	return glm::normalize(glm::vec3(my_mouse_in_world_space));
}

void Camera::CalculateFrustumVertices()
{
	double PI = 3.14159265;
	double aspectRatio = windowWidth / windowHeight;
	//double heightNear = 2.0 * tan(MathUtils::ToRadians(camera->fov * 0.5)) * camera->near;
	//double heightNear = 2.0 * tan(MathUtils::ToRadians(fov * 0.5)) * (-shadowNearOffset);
	double heightNear = tan(glm::radians(fov * 0.5)) * (-shadowNearOffset);
	double widthNear = heightNear * aspectRatio;
	//double heightFar = 2.0 * tan(MathUtils::ToRadians(camera->fov * 0.5)) * camera->far;
	//double heightFar = 2.0 * tan(MathUtils::ToRadians(fov * 0.5)) * shadowDistance;
	double heightFar = tan(glm::radians(fov * 0.5)) * shadowDistance;
	double widthFar = heightFar * aspectRatio;
	//Vector3 nearPlane = camera->GetPosition() + camera->forward * camera->near;
	glm::vec3 nearPlane = GetPosition() + forward * (-shadowNearOffset);
	//Vector3 farPlane = camera->GetPosition() + camera->forward * camera->far;
	glm::vec3 farPlane = GetPosition() + forward * shadowDistance;

	glm::vec3 heightNearUp = up * (float)heightNear;
	glm::vec3 heightFarUp = up * (float)heightFar;
	glm::vec3 widthNearRight = right * (float)widthNear;
	glm::vec3 widthFarRight = right * (float)widthFar;
	frustumVertices[0] = nearPlane + heightNearUp - widthNearRight; //nearUpperLeftCorner
	frustumVertices[1] = nearPlane + heightNearUp + widthNearRight; //nearUpperRightCorner
	frustumVertices[2] = nearPlane - heightNearUp - widthNearRight; //nearLowerLeftCorner
	frustumVertices[3] = nearPlane - heightNearUp + widthNearRight; //nearLowerRightCorner
	frustumVertices[4] = farPlane + heightFarUp - widthFarRight; //farUpperLeftCorner
	frustumVertices[5] = farPlane + heightFarUp + widthFarRight; //farUpperRightCorner
	frustumVertices[6] = farPlane - heightFarUp - widthFarRight; //farLowerLeftCorner
	frustumVertices[7] = farPlane - heightFarUp + widthFarRight; //farLowerRightCorner
	frustumVertices[8] = nearPlane + (farPlane - nearPlane); //frustumCenter
	frustumVertices[9] = GetPosition(); //frustumOrigin
}

void Camera::ComputeVectors()
{
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	forward.x = cos(verticalAngle) * sin(horizontalAngle);
	forward.y = sin(verticalAngle);
	forward.z = cos(verticalAngle) * cos(horizontalAngle);

	// Right vector
	right.x = sin(horizontalAngle - 3.14 / 2.0),
	right.y = 0.0;
	right.z = cos(horizontalAngle - 3.14 / 2.0);

	// Up vector
	up = glm::cross(right, forward);

	forward = glm::normalize(forward);
	right = glm::normalize(right);
	up = glm::normalize(up);
}

void Camera::CalculateViewMatrix()
{
	ViewMatrix = glm::lookAt(position, position + forward, up);
	ViewInverseMatrix = glm::inverse(ViewMatrix);
}

void Camera::UpdateProjection()
{
	ProjectionMatrix = glm::perspective(glm::radians(fov), (float)windowWidth / (float)windowHeight, near, far);
	ProjectionInverseMatrix = glm::inverse(ProjectionMatrix);
}

void Camera::SetPosition(const glm::vec3& pos)
{
	//proper way to set position would be to get the position from matrix via inverse
	//then new position - camera pos -> vector with direction to new position 
	//then we translate the initial position with that vector -> initialPosition += new vector with dir
	//this would be required if one did change the camera matrix directly rather than changing the position variable 
	position = pos;
}

void Camera::SetTarget(const glm::vec3& target)
{
	this->target = target;
}


