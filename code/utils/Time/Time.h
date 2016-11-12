#pragma once
namespace mwm
{ 
class Time
{
public:
	Time();
	~Time();
	static double timeStep; //physics
	static double deltaTime;
	static double dtInv;
	static double timeModifier;
	static double currentTime;
	static double previousTime;
private:

};

}