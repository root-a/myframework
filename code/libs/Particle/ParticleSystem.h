#pragma once
#include "MyMathLib.h"
#include "Component.h"
#include <vector>

class Particle;
class Material;
struct ParticleData;
struct ParticleInfo;
class VertexBufferDynamic;
class LocationLayout;
class VertexArray;

class ParticleSystem : public Component
{
public:
	typedef unsigned int GLuint;
	ParticleSystem();
	ParticleSystem(int maxParticles, int emissionRate);
	~ParticleSystem();
	Component* Clone();
	std::vector<ParticleData> ParticlesData;
	std::vector<ParticleInfo> ParticlesInfo;
	//int FindUnusedParticle();
	void UpdateParticles(double deltaTime);
	void SortParticles();
	void UpdateBuffers();
	int Draw();
	void SetTexture(GLuint textureID);
	void SetEmissionRate(int emissionRate);
	void SetMaxParticles(int maxParticles);
	void SetColor(const glm::vec4& color);
	void SetSize(float size);
	void SetLifeTime(float lifetime);
	void SetDirection(const glm::vec3& direction);
	void SetSpread(float spread);
	void SetForce(const glm::vec3& force);
	void SetAdditive(bool isAdditiveMode);
	int GetAliveParticlesCount();
	int GetNewParticlesCount();
	void Update();
	static const glm::vec3 g_vertex_buffer_data[4];
	static const unsigned char elements[6];
	inline void NewParticle(int index);
	inline void UpdateParticle(int index);
	inline void CalculateNewEmitedParticles(double deltaTime);

	VertexBufferDynamic* particles_data_buffer;
	GLuint TextureID;

	GLuint CameraRightHandle;
	GLuint CameraUpHandle;
	GLuint ViewProjectionHandle;
	GLuint TextureSamplerHandle;

	bool additive;
	bool paused;
	VertexArray* vao;
	int MaxParticles;
	int EmissionRate;
	int DesiredEmissionRate;
	int LastParticleIndex = 0;
	int DeadParticles = 0;
	int ReallyAliveParticles = 0;
	int NewParticles = 0;
	float LifeTime;
	float Size;
	float Spread;
	glm::vec3 Force;
	glm::vec3 Direction;
	glm::vec4 Color;
	void SetUp();
protected:
private:
	int LastUsedParticle;
	double timeSinceLastEmission;
};

