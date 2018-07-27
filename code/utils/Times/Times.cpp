#include "Times.h"

namespace mwm
{
Times::Times()
{
	timeModifier = 0.0;
	timeStep = 0.016 + timeModifier;
	dtInv = 1.0 / timeStep;
	paused = false;
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
	currentTime = currentTimeIn;
	
	timeStep = 0.016 + timeModifier;
	if (timeStep == 0.0) dtInv = 0.0;
	else dtInv = 1.0 / timeStep;
	if (paused) timeStep = 0.0, dtInv = 0.0;
}

}
