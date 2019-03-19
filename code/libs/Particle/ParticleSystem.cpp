#include "ParticleSystem.h"
#include <algorithm>
#include "Gl/glew.h"
#include "Particle.h"
#include "Object.h"
#include "Times.h"
#include "CameraManager.h"
#include "Camera.h"
#include <cstddef>
using namespace mwm;
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
	dynamic = true;
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

void ParticleSystem::UpdateParticles(double deltaTime, const mwm::Vector3& camPos)
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

	glGenVertexArrays(1, &vao.vaoHandle);
	glBindVertexArray(vao.vaoHandle);

	vao.vertexBuffers.reserve(3);

	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3F), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	vao.vertexBuffers.push_back(billboard_vertex_buffer);
	
	vao.indicesCount = 6;
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indicesCount * sizeof(GLushort), elements, GL_STATIC_DRAW);
	vao.vertexBuffers.push_back(elementbuffer);

	// The VBO containing the data of the particles
	glGenBuffers(1, &particles_data_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(ParticleData), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)offsetof(ParticleData, ParticleData::color));
	vao.vertexBuffers.push_back(particles_data_buffer);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	glBindVertexArray(0);
}

void ParticleSystem::UpdateBuffers()
{
	//Bind VAO
	glBindVertexArray(vao.vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, particles_data_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), g_particule_position_size_data, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(ParticleData), g_particule_data);
}

int ParticleSystem::Draw(Matrix4F& ViewProjection, GLuint currentShaderID, const Vector3F& cameraUp, const Vector3F& cameraRight)
{
	UpdateBuffers();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	// Use additive blending to give it a 'glow' effect
	if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CameraRightHandle = glGetUniformLocation(currentShaderID, "CameraRight");
	CameraUpHandle = glGetUniformLocation(currentShaderID, "CameraUp");
	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");

	glUniform3fv(CameraRightHandle, 1, &cameraRight.x);
	glUniform3fv(CameraUpHandle, 1, & cameraUp.x);

	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glDrawElementsInstanced(GL_TRIANGLES, vao.indicesCount, GL_UNSIGNED_SHORT, (void*)0, aliveParticles);

	if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_BLEND);
	
	return aliveParticles;
}

void ParticleSystem::SetTexture(GLuint textureID)
{
	this->TextureID = textureID;
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

void ParticleSystem::SetColor(const mwm::Vector4F& color)
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

void ParticleSystem::SetDirection(const mwm::Vector3F& direction)
{
	Direction = direction;
}

void ParticleSystem::SetSpread(float spread)
{
	Spread = spread;
}

void ParticleSystem::SetForce(mwm::Vector3 & force)
{
	Force = force;
}
