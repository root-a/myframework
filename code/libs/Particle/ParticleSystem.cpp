#include "ParticleSystem.h"
#include <algorithm>
#include "Gl/glew.h"
#include "Particle.h"
#include "Object.h"
#include "Time.h"
#include "CameraManager.h"
#include "Camera.h"
#include "ShaderManager.h"

using namespace mwm;

const Vector3F ParticleSystem::g_vertex_buffer_data[4] = {
	Vector3F(-0.5f, -0.5f, 0.0f),
	Vector3F(0.5f, -0.5f, 0.0f),
	Vector3F(-0.5f, 0.5f, 0.0f),
	Vector3F(0.5f, 0.5f, 0.0f),
};

ParticleSystem::ParticleSystem(int MaxParticles, int emissionRate)
{
	this->LastUsedParticle = 0;
	this->MaxParticles = MaxParticles;
	ParticlesContainer = new Particle[MaxParticles];
	g_particule_position_size_data = new Vector4F[MaxParticles];
	g_particule_color_data = new Vector4F[MaxParticles];
	for (int i = 0; i < MaxParticles; i++){
		ParticlesContainer[i].lifeTime = -1.0f;
		ParticlesContainer[i].cameraDistance = -1.0;
	}
	EmissionRate = emissionRate;
	Color = Vector4F(1.f, 1.f, 1.f, 0.8f);
	Size = 1.f;
	LifeTime = 0.2f;
	Direction = Vector3F(0.0f, 10.0f, 0.0f);
	Spread = 1.5f;
	aliveParticles = 0;
	SetUp();
}

ParticleSystem::~ParticleSystem()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoHandle);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &particles_color_buffer);
	delete[] ParticlesContainer;
	delete[] g_particule_position_size_data;
	delete[] g_particule_color_data;
}

int ParticleSystem::FindUnusedParticle()
{
	for (int i = LastUsedParticle; i < MaxParticles; i++){
		if (ParticlesContainer[i].lifeTime < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++){
		if (ParticlesContainer[i].lifeTime < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; 
}

int ParticleSystem::UpdateParticles(double deltaTime, const mwm::Vector3& camPos)
{
	int ParticlesCount = 0;
	for (int i = 0; i < MaxParticles; i++){

		Particle& p = ParticlesContainer[i]; 

		if (p.lifeTime > 0.0f)
		{
			p.lifeTime -= (float)deltaTime;

			// Simulate simple physics : gravity only, no collisions
			p.speed += Vector3(0.0, -9.81, 0.0) * deltaTime * 0.5;
			p.pos += p.speed * deltaTime;
			p.cameraDistance = (p.pos - camPos).vectLengt();
			//p.size -= (float)deltaTime*3.5f;
			//if (p.size < 0.f) p.size = 0.f; p.lifeTime = 0.f;
			// Fill the GPU buffer
			g_particule_position_size_data[ParticlesCount] = Vector4F(p.pos.toFloat(), p.size);
			g_particule_color_data[ParticlesCount] = p.color;

			ParticlesCount++;
		}
		else{
			// Particles that just died will be put at the end of the buffer in SortParticles();
			p.cameraDistance = -1.0;
		}
	}
	return ParticlesCount;
}

void ParticleSystem::SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void ParticleSystem::SetUp()
{
	//Create VAO
	glGenVertexArrays(1, &vaoHandle);
	//Bind VAO
	glBindVertexArray(vaoHandle);

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3F), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	
	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4F), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	//Unbind the VAO now that the VBOs have been set up
	glBindVertexArray(0);
}

void ParticleSystem::UpdateBuffers()
{
	//Bind VAO
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(Vector4), g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(Vector4), g_particule_color_data);
}

void ParticleSystem::Draw(const Matrix4& ViewProjection, GLuint currentShaderID, const Vector3F& cameraUp, const Vector3F& cameraRight)
{
	UpdateBuffers();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	// Use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CameraRightHandle = glGetUniformLocation(currentShaderID, "CameraRight");
	CameraUpHandle = glGetUniformLocation(currentShaderID, "CameraUp");
	ViewProjectionHandle = glGetUniformLocation(currentShaderID, "VP");
	// Same as the billboards tutorial

	glUniform3fv(CameraRightHandle, 1, &cameraRight.x);
	glUniform3fv(CameraUpHandle, 1, & cameraUp.x);

	glUniformMatrix4fv(ViewProjectionHandle, 1, GL_FALSE, &ViewProjection.toFloat()[0][0]);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, aliveParticles);

	// Don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_BLEND);
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
	int newparticles = (int)(deltaTime*EmissionRate);
	if (newparticles > (int)(deltaTime*EmissionRate))
		newparticles = (int)(deltaTime*EmissionRate);

	for (int i = 0; i < newparticles; i++){
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].lifeTime = LifeTime; // This particle will live 5 seconds.
		ParticlesContainer[particleIndex].pos = object->GetWorldPosition(); //Vector3();

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
	GenerateNewParticles(Time::deltaTime);
	aliveParticles = UpdateParticles(Time::deltaTime, CameraManager::Instance()->GetCurrentCamera()->GetPosition2());
	SortParticles();
}

void ParticleSystem::SetLifeTime(float lifetime)
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
