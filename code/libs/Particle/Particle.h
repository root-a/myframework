#pragma once
#include "MyMathLib.h"
struct ParticleData
{
	mwm::Vector4F posAndSize;
	mwm::Vector4F color;
};

class Particle{
public:
	Particle();
	~Particle();
	
	mwm::Vector3 pos, speed;
	mwm::Vector4F color;
	float size, angle, weight;
	double lifeTime;
	double cameraDistance;

	bool operator<(Particle& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
};

