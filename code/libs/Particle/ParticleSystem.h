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
	void Draw(const mwm::Matrix4& ViewProjection, GLuint currentShaderID, const mwm::Vector3& cameraUp, const mwm::Vector3& cameraRight);
	void SetTexture(GLuint textureID);
	void SetEmissionRate(int emissionRate);
	void SetColor(const mwm::Vector4& color);
	void Update();
	static const mwm::Vector3 g_vertex_buffer_data[4];
	mwm::Vector4* g_particule_position_size_data;
	mwm::Vector4* g_particule_color_data;

	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	GLuint vaoHandle;

	GLuint TextureID;

	GLuint CameraRightHandle;
	GLuint CameraUpHandle;
	GLuint ViewProjectionHandle;
	GLuint TextureSamplerHandle;
	void* operator new(size_t i) { return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }

protected:
private:
	int LastUsedParticle;
	int MaxParticles;
	int EmissionRate;
	mwm::Vector4 Color;
	void SetUp();
	int aliveParticles;
};

