#pragma once

class Camera
{
public:
	Camera(const mwm::Vector3& initPos, int windowWidth, int windowHeight, double newNearPlane = 0.1, double newFarPlane = 100.0, double newFov = 45.0);
	~Camera();
	void Update(double deltaTime);

	void CalculateViewMatrix();

	void UpdateProjection();

	void ComputeVectors();

	void UpdatePosition(double deltaTime);

	void UpdateOrientation(double mouseX, double mouseY);

	void UpdateSize(int width, int height);

	mwm::Vector3 ConvertMousePosToWorldDir(double mousePosX, double mousePosY);

	mwm::Vector3 GetInitPos();
	mwm::Vector3 GetPosition();
	mwm::Vector3 GetPosition2();

	void SetPosition(mwm::Vector3& pos);

	bool holdingForward;
	bool holdingBackward;
	bool holdingRight;
	bool holdingLeft;
	bool holdingUp;
	bool holdingDown;

	mwm::Matrix4 ProjectionMatrix;
	mwm::Matrix4 ViewMatrix;
	int windowWidth;
	int windowHeight;
	double windowMidX;
	double windowMidY;

	double fov;
	double near;
	double far;
	mwm::Vector3 direction;
	mwm::Vector3 up;
	mwm::Vector3 right;
private:
	mwm::Vector3 position;
	double verticalAngle;
	double horizontalAngle;

	double speed;
	double mouseSpeed;
};