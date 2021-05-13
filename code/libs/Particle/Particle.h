#pragma once
#include "MyMathLib.h"
struct ParticleData
{
	Vector4F posAndSize;
	Vector4F color;
};

class Particle{
public:
	Particle();
	~Particle();
	
	Vector3 pos, speed;
	Vector4F color;
	float size, angle, weight;
	double lifeTime;
	double cameraDistance;

	bool operator<(Particle& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
};

