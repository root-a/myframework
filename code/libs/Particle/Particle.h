#pragma once
#include "MyMathLib.h"
struct ParticleData
{
	glm::vec3 pos;
	float size;
	glm::vec4 color;
};

struct ParticleInfo
{
	glm::vec3 speed;
	float angle, weight;
	double lifeTime = -1.0;
	double cameraDistance = -1.0;
};

class Particle{
public:
	Particle();
	~Particle();
	ParticleData data;
	ParticleInfo info;

	bool operator<(Particle& that){
		// Sort in reverse order : far particles drawn first.
		return this->info.cameraDistance > that.info.cameraDistance;
	}
};

