#pragma once
#include "MyMathLib.h"
#include "Component.h"

class Particle;
class Material;

class ParticleSystem : public Component
{
public:
	typedef unsigned int GLuint;
	ParticleSystem(int maxParticles, int emissionRate);
	~ParticleSystem();
	
	Particle* ParticlesContainer;
	int FindUnusedParticle();
	void GenerateNewParticles(double deltaTime);
	int UpdateParticles(double deltaTime, const mwm::Vector3& camPos);
	void SortParticles();
	void UpdateBuffers();
	void Draw(mwm::Matrix4F& ViewProjection, GLuint currentShaderID, const mwm::Vector3F& cameraUp, const mwm::Vector3F& cameraRight);
	void SetTexture(GLuint textureID);
	void SetEmissionRate(int emissionRate);
	void SetColor(const mwm::Vector4F& color);
	void SetSize(float size);
	void SetLifeTime(float lifetime);
	void SetDirection(const mwm::Vector3F& direction);
	void SetSpread(float spread);
	void Update();
	static const mwm::Vector3F g_vertex_buffer_data[4];
	mwm::Vector4F* g_particule_position_size_data;
	mwm::Vector4F* g_particule_color_data;

	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	GLuint vaoHandle;

	GLuint TextureID;

	GLuint CameraRightHandle;
	GLuint CameraUpHandle;
	GLuint ViewProjectionHandle;
	GLuint TextureSamplerHandle;

protected:
private:
	int LastUsedParticle;
	int MaxParticles;
	int EmissionRate;
	mwm::Vector4F Color;
	void SetUp();
	int aliveParticles;
	float Size;
	float LifeTime;
	mwm::Vector3F Direction;
	float Spread;
};

