#pragma once
#include <string>
#include "Vector3.h"
#include "Vector2F.h"
class Camera
{
public:
	enum class FrustumVertices
	{
		nearUpperLeftCorner,
		nearUpperRightCorner,
		nearLowerLeftCorner,
		nearLowerRightCorner,
		farUpperLeftCorner,
		farUpperRightCorner,
		farLowerLeftCorner,
		farLowerRightCorner,
		frustumCenter,
		frustumOrigin
	};
	Camera(const glm::vec3& initPos = glm::vec3(0,0,0), int windowWidth = 1024, int windowHeight = 768, double newNearPlane = 0.1, double newFarPlane = 2000.0, double newFov = 45.0);
	~Camera();
	void Update(double deltaTime);

	void CalculateViewMatrix();

	void UpdateProjection();

	void ComputeVectors();

	void UpdatePosition(double deltaTime);

	void UpdateOrientation(double mouseX, double mouseY);

	void UpdateSize(int width, int height);

	glm::vec3 ConvertMousePosToWorldDir(double mousePosX, double mousePosY);

	void CalculateFrustumVertices();

	glm::vec3 GetInitPos();
	glm::vec3 GetPosition();
	glm::vec3 GetPosition2();

	void SetPosition(const glm::vec3& pos);
	void SetTarget(const glm::vec3& target);

	bool holdingForward;
	bool holdingBackward;
	bool holdingRight;
	bool holdingLeft;
	bool holdingUp;
	bool holdingDown;
	float shadowNearOffset;
	float shadowDistance;
	glm::mat4 ProjectionMatrix;
	glm::mat4 ProjectionInverseMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ViewInverseMatrix;
	union
	{
		struct { float windowWidth, windowHeight; };
		glm::vec2 screenSize;
	};
	
	double windowMidX;
	double windowMidY;

	float fov;
	float near;
	float far;
	glm::vec3 target;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 forward;
	float speed;
	std::string name;
	double verticalAngle;
	double horizontalAngle;
	glm::vec3 frustumVertices[10];
private:
	glm::vec3 position;
	double mouseSpeed;
};