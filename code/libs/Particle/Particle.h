#pragma once
#include "MyMathLib.h"

class Particle{
public:
	Particle();
	~Particle();

	mwm::Vector3 pos, speed;
	mwm::Vector4 color;
	float size, angle, weight;
	float lifeTime;
	float cameraDistance;

	bool operator<(Particle& that){
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}

	void* operator new(size_t i){ return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
};

