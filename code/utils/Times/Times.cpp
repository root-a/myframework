#include "Times.h"

Times::Times()
{
	timeModifier = 0.0;
	timeStep = 0.016 + timeModifier;
	dtInv = 1.0 / timeStep;
	paused = false;
	timeModifierF = (float)timeModifier;
	timeStepF = (float)timeStep;
	dtInvF = (float)dtInv;
	averageFPSafterNFrames = 100;
	frameCount = 0;
}

Times::~Times()
{
}

Times * Times::Instance()
{
	static Times instance;

	return &instance;
}

void
Times::Update(double currentTimeIn)
{
	deltaTime = currentTimeIn - currentTime;
	deltaSum += deltaTime;
	if (frameCount >= averageFPSafterNFrames)
	{
		averageDeltaTime = deltaSum / averageFPSafterNFrames;
		frameCount = 0;
		deltaSum = 0.0;
	}
	else
	{
		frameCount++;
	}

	previousTime = currentTime;
	currentTime = currentTimeIn;
	
	timeStep = 0.016 + timeModifier;
	if (timeStep == 0.0) dtInv = 0.0;
	else dtInv = 1.0 / timeStep;
	if (paused) timeStep = 0.0, dtInv = 0.0;

	deltaTimeF = (float)deltaTime;
	currentTimeF = (float)currentTime;
	timeStepF = (float)timeStep;
	timeModifierF = (float)timeModifier;
	dtInvF = (float)dtInv;
	previousTimeF = (float)previousTime;
	
}