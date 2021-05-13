#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include "Vao.h"

class Particle;
class Material;
struct ParticleData;

class ParticleSystem : public Component
{
public:
	typedef unsigned int GLuint;
	ParticleSystem(int maxParticles, int emissionRate);
	~ParticleSystem();
	
	Particle* ParticlesContainer;
	int FindUnusedParticle();
	void GenerateNewParticles(double deltaTime);
	void UpdateParticles(double deltaTime, const Vector3& camPos);
	void SortParticles();
	void UpdateBuffers();
	int Draw();
	void SetTexture(GLuint textureID);
	void SetEmissionRate(int emissionRate);
	void SetColor(const Vector4F& color);
	void SetSize(float size);
	void SetLifeTime(double lifetime);
	void SetDirection(const Vector3F& direction);
	void SetSpread(float spread);
	void SetForce(Vector3& force);
	void SetAdditive(bool isAdditiveMode);
	void Update();
	static const Vector3F g_vertex_buffer_data[4];
	static const unsigned short elements[6];
	ParticleData* g_particule_data;

	GLuint particles_data_buffer;
	GLuint TextureID;

	GLuint CameraRightHandle;
	GLuint CameraUpHandle;
	GLuint ViewProjectionHandle;
	GLuint TextureSamplerHandle;

	bool additive;
	bool paused;
	Vao vao;
protected:
private:
	int LastUsedParticle;
	int MaxParticles;
	int EmissionRate;
	Vector4F Color;
	void SetUp();
	int aliveParticles;
	int newparticles;
	float Size;
	double LifeTime;
	Vector3F Direction;
	float Spread;
	Vector3 Force;
};

