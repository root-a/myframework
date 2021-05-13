#include "ParticleSystem.h"
#include <algorithm>
#include "Gl/glew.h"
#include "Particle.h"
#include "Object.h"
#include "Times.h"
#include "CameraManager.h"
#include "Camera.h"
#include <cstddef>

//vertices are mathematically clockwise
const Vector3F ParticleSystem::g_vertex_buffer_data[4] = {
	Vector3F(-0.5f, -0.5f, 0.0f),
	Vector3F(-0.5f, 0.5f, 0.0f),
	Vector3F(0.5f, 0.5f, 0.0f),
	Vector3F(0.5f, -0.5f, 0.0f),
};
//indices are mathematically counter-clockwise
const GLushort ParticleSystem::elements[] = {
	0, 3, 2,
	2, 1, 0
};

ParticleSystem::ParticleSystem(int MaxParticles, int emissionRate)
{
	this->LastUsedParticle = 0;
	this->MaxParticles = MaxParticles;
	ParticlesContainer = new Particle[MaxParticles];
	g_particule_data = new ParticleData[MaxParticles];
	for (int i = 0; i < MaxParticles; i++){
		ParticlesContainer[i].lifeTime = -1.0f;
		ParticlesContainer[i].cameraDistance = -1.0;
	}
	EmissionRate = emissionRate;
	Color = Vector4F(1.f, 1.f, 1.f, 0.8f);
	Size = 1.f;
	LifeTime = 0.2;
	Direction = Vector3F(0.0f, 10.0f, 0.0f);
	Spread = 1.5f;
	aliveParticles = 0;
	SetUp();
	additive = true;
	Force = Vector3(0.0, -9.81, 0.0);
	paused = false;
}

ParticleSystem::~ParticleSystem()
{
	delete[] ParticlesContainer;
	delete[] g_particule_data;
}

int ParticleSystem::FindUnusedParticle()
{
	int particleWithLowestLifeTime = 0;
	double lowestParticleLifeTime = 10000.0;
	for (int i = LastUsedParticle; i < MaxParticles; i++){
		if (ParticlesContainer[i].lifeTime < 0.0){
			LastUsedParticle = i;
			return i;
		}
		else if(ParticlesContainer[i].lifeTime < lowestParticleLifeTime)
		{
			particleWithLowestLifeTime = i;
			lowestParticleLifeTime = ParticlesContainer[i].lifeTime;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++){
		if (ParticlesContainer[i].lifeTime < 0.0){
			LastUsedParticle = i;
			return i;
		}
		else if (ParticlesContainer[i].lifeTime < lowestParticleLifeTime)
		{
			particleWithLowestLifeTime = i;
			lowestParticleLifeTime = ParticlesContainer[i].lifeTime;
		}
	}

	return particleWithLowestLifeTime;
}

void ParticleSystem::UpdateParticles(double deltaTime, const Vector3& camPos)
{
	aliveParticles = 0;
	for (int i = 0; i < MaxParticles; i++){

		Particle& p = ParticlesContainer[i]; 

		if (p.lifeTime < 0.0)
		{
			// Particles that just died will be put at the end of the buffer in SortParticles();
			p.cameraDistance = -1.0;
		}
		else{
			p.lifeTime -= deltaTime;

			// Simulate simple physics : gravity only, no collisions
			p.speed += Force * deltaTime * 0.5;
			p.pos += p.speed * deltaTime;
			p.cameraDistance = (p.pos - camPos).squareMag();
			//p.size -= (float)deltaTime*3.5f;
			//if (p.size < 0.f) p.size = 0.f; p.lifeTime = 0.f;
			// Fill the GPU buffer
			g_particule_data[aliveParticles].posAndSize = Vector4F(p.pos.toFloat(), p.size);
			g_particule_data[aliveParticles].color = p.color;

			aliveParticles++;
		}
	}
}

void ParticleSystem::SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void ParticleSystem::SetUp()
{
	vao.vertexBuffers.reserve(3);
	vao.AddVertexBuffer(g_vertex_buffer_data, sizeof(g_vertex_buffer_data), { {ShaderDataType::Float3, "Vertex"} });
	particles_data_buffer = vao.AddVertexBuffer(NULL, MaxParticles * sizeof(ParticleData), { {ShaderDataType::Float4, "CenterPositionSize", 1}, {ShaderDataType::Float4, "Color", 1} });
	vao.AddIndexBuffer(elements, 6, IndicesType::UNSIGNED_SHORT);
}

void ParticleSystem::UpdateBuffers()
{
	glNamedBufferSubData(particles_data_buffer, 0, aliveParticles * sizeof(ParticleData), g_particule_data);
}

int ParticleSystem::Draw()
{
	UpdateBuffers();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	// Use additive blending to give it a 'glow' effect
	if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	vao.activeCount = aliveParticles;
	vao.Bind();
	vao.Draw();

	if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_BLEND);
	
	return aliveParticles;
}

void ParticleSystem::SetTexture(GLuint textureID)
{
	this->TextureID = textureID;
}

void ParticleSystem::SetAdditive(bool isAdditiveMode)
{
	additive = isAdditiveMode;
}

void ParticleSystem::GenerateNewParticles(double deltaTime)
{
	// Generate 10 new particule each millisecond,
	// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
	// newparticles will be huge and the next frame even longer.
	newparticles = ceil(deltaTime*EmissionRate);
	if (newparticles > (int)(0.016*EmissionRate))
		newparticles = (int)(0.016*EmissionRate);

	for (int i = 0; i < newparticles; i++){
		int particleIndex = FindUnusedParticle();
		
		ParticlesContainer[particleIndex].lifeTime = LifeTime; // This particle will live 5 seconds.
		ParticlesContainer[particleIndex].pos = object->node->GetWorldPosition(); //Vector3();
		//printf("%d p: %f %f %f\n", particleIndex, ParticlesContainer[particleIndex].pos.x, ParticlesContainer[particleIndex].pos.y, ParticlesContainer[particleIndex].pos.z);
		
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		Vector3F randomdir = Vector3F(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
			);
		randomdir = Direction + randomdir*Spread;
		ParticlesContainer[particleIndex].speed = Vector3(randomdir.x, randomdir.y, randomdir.z);
		ParticlesContainer[particleIndex].color = Color;

		ParticlesContainer[particleIndex].size = Size;//(float)((rand() % 1000) / 2000.0f + 0.1f);
	}
}

void ParticleSystem::SetEmissionRate(int emissionRate)
{
	EmissionRate = emissionRate;
}

void ParticleSystem::SetColor(const Vector4F& color)
{
	Color = color;
}

void ParticleSystem::SetSize(float size)
{
	Size = size;
}

void ParticleSystem::Update()
{
	if (!paused)
	{
		GenerateNewParticles(Times::Instance()->deltaTime);
		UpdateParticles(Times::Instance()->deltaTime, CameraManager::Instance()->GetCurrentCamera()->GetPosition2());
		if (!additive) SortParticles();
	}
}

void ParticleSystem::SetLifeTime(double lifetime)
{
	LifeTime = lifetime;
}

void ParticleSystem::SetDirection(const Vector3F& direction)
{
	Direction = direction;
}

void ParticleSystem::SetSpread(float spread)
{
	Spread = spread;
}

void ParticleSystem::SetForce(Vector3 & force)
{
	Force = force;
}
