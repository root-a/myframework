#pragma once

class Camera
{
public:
	Camera(const mwm::Vector3& initPos, int windowWidth, int windowHeight);
	~Camera();
	void Update(double deltaTime);

	void CalculateViewMatrix();

	void UpdateProjection();

	void ComputeVectors();

	void UpdatePosition(double deltaTime);

	void UpdateOrientation(double mouseX, double mouseY);

	void UpdateSize(int width, int height);

	void SetFoV(double newFov);
	
	void SetNearPlane(double newNearPlane);
	
	void SetFarPlane(double newFarPlane);

	mwm::Vector3 getDirection();
	mwm::Vector3 getUp();
	mwm::Vector3 getRight();

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
private:
	
	mwm::Vector3 direction;
	mwm::Vector3 up;
	mwm::Vector3 right;
	mwm::Vector3 position;

	int windowWidth;
	int windowHeight;
	double windowMidX;
	double windowMidY;

	// Initial horizontal angle : toward -Z
	double horizontalAngle;
	// Initial vertical angle : none
	double verticalAngle;

	double speed;
	double mouseSpeed;

	double fov;
	double near;
	double far;
};
