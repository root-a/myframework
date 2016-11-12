#include "Time.h"

namespace mwm
{
Time::Time()
{
	timeStep = 0.016 + timeModifier;
	dtInv = 1.0 / timeStep;
}

Time::~Time()
{
}
double Time::timeStep;
double Time::deltaTime;
double Time::dtInv;
double Time::timeModifier;
double Time::currentTime;
double Time::previousTime;
}
