#pragma once

class Times
{
public:
	Times();
	~Times();
	static Times* Instance();
	void Update(double currentTimeIn);
	double timeStep; //physics
	double deltaTime;
	double averageDeltaTime;
	int averageFPSafterNFrames;
	double dtInv;
	double timeModifier;
	double currentTime;
	double previousTime;
	float timeStepF;
	float deltaTimeF;
	float dtInvF;
	float timeModifierF;
	float currentTimeF;
	float previousTimeF;
	bool paused;
private:
	int frameCount;
	double deltaSum;
};