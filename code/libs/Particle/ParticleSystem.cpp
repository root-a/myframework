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

const Vector3 ParticleSystem::g_vertex_buffer_data[4] = {
	Vector3(-0.5f, -0.5f, 0.0f),
	Vector3(0.5f, -0.5f, 0.0f),
	Vector3(-0.5f, 0.5f, 0.0f),
	Vector3(0.5f, 0.5f, 0.0f),
};

ParticleSystem::ParticleSystem(int MaxParticles, int emissionRate)
{
	this->LastUsedParticle = 0;
	this->MaxParticles = MaxParticles;
	ParticlesContainer = new Particle[MaxParticles];
	g_particule_position_size_data = new Vector4[MaxParticles];
	g_particule_color_data = new Vector4[MaxParticles];
	for (int i = 0; i < MaxParticles; i++){
		ParticlesContainer[i].lifeTime = -1.0f;
		ParticlesContainer[i].cameraDistance = -1.0f;
	}
	EmissionRate = emissionRate;
	Color = Vector4(1.f, 1.f, 1.f, 0.8f);
	SetUp();
}

ParticleSystem::~ParticleSystem()
{
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
			//p.speed += Vector3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
			//p.pos += p.speed * (float)deltaTime;
			p.cameraDistance = (p.pos - camPos).vectLengthSSE();
			p.size -= (float)deltaTime*3.5f;
			if (p.size < 0.f) p.size = 0.f;
			// Fill the GPU buffer
			g_particule_position_size_data[ParticlesCount] = Vector4(p.pos, p.size);
			g_particule_color_data[ParticlesCount] = p.color;

			ParticlesCount++;
		}
		else{
			// Particles that just died will be put at the end of the buffer in SortParticles();
			p.cameraDistance = -1.0f;
		}
	}
	return ParticlesCount;
}

void ParticleSystem::SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void ParticleSystem::SetUp()
{
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	
	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
}

void ParticleSystem::UpdateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(Vector4), g_particule_position_size_data);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(Vector4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(Vector4), g_particule_color_data);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
}

void ParticleSystem::Draw(const Matrix4& ViewProjection, GLuint currentShaderID, const Vector3& cameraUp, const Vector3& cameraRight)
{
	UpdateBuffers();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, aliveParticles);

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
	if (newparticles > (int)(0.016f*EmissionRate))
		newparticles = (int)(0.016f*EmissionRate);

	for (int i = 0; i < newparticles; i++){
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].lifeTime = 0.2f; // This particle will live 5 seconds.
		ParticlesContainer[particleIndex].pos = object->GetWorldPosition(); //Vector3();

		float spread = 1.5f;
		Vector3 maindir = Vector3(0.0f, 10.0f, 0.0f);
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		Vector3 randomdir = Vector3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
			);

		ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;
		ParticlesContainer[particleIndex].color = Color;

		ParticlesContainer[particleIndex].size = 1.f;//(float)((rand() % 1000) / 2000.0f + 0.1f);
	}
}

void ParticleSystem::SetEmissionRate(int emissionRate)
{
	EmissionRate = emissionRate;
}

void ParticleSystem::SetColor(const mwm::Vector4& color)
{
	Color = color;
}

void ParticleSystem::Update()
{
	GenerateNewParticles(Time::timeStep);
	aliveParticles = UpdateParticles(Time::timeStep, CameraManager::Instance()->GetCurrentCamera()->GetPosition2());
	SortParticles();
}

