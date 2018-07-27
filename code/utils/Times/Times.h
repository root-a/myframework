#pragma once
namespace mwm
{ 
class Times
{
public:
	Times();
	~Times();
	static Times* Instance();
	void Update(double currentTimeIn);
	double timeStep; //physics
	double deltaTime;
	double dtInv;
	double timeModifier;
	double currentTime;
	double previousTime;
	bool paused;
private:

};

}