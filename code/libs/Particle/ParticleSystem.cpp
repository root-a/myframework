#include "ParticleSystem.h"
#include <algorithm>
#include "Gl/glew.h"
#include "Particle.h"
#include "Object.h"
#include "Times.h"
#include "CameraManager.h"
#include "Camera.h"
#include <cstddef>
#include "Vao.h"
#include "GraphicsStorage.h"

//OBJ
//vertices are mathematically clockwise
const glm::vec3 ParticleSystem::g_vertex_buffer_data[4] = {
	glm::vec3(-0.5f, -0.5f, 0.0f),
	glm::vec3(-0.5f, 0.5f, 0.0f),
	glm::vec3(0.5f, 0.5f, 0.0f),
	glm::vec3(0.5f, -0.5f, 0.0f),
};

//OBJ
//indices are mathematically counter-clockwise
const GLubyte ParticleSystem::elements[] = {
	0, 3, 2,
	2, 1, 0
};

ParticleSystem::ParticleSystem()
{
	LastUsedParticle = 0;
	Color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
	Size = 1.f;
	LifeTime = 0.2;
	Direction = glm::vec3(0.0f, 10.0f, 0.0f);
	Spread = 1.5f;
	additive = true;
	Force = glm::vec3(0.0, -9.81, 0.0);
	paused = false;
	timeSinceLastEmission = 0.0;
	vao = nullptr;
}

ParticleSystem::ParticleSystem(int maxParticles, int emissionRate)
{
	LastUsedParticle = 0;
	MaxParticles = maxParticles;
	ParticlesData.resize(maxParticles);
	ParticlesInfo.resize(maxParticles);
	DesiredEmissionRate = emissionRate;
	Color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
	Size = 1.f;
	LifeTime = 0.2;
	Direction = glm::vec3(0.0f, 10.0f, 0.0f);
	Spread = 1.5f;
	vao = nullptr;
	additive = true;
	Force = glm::vec3(0.0, -9.81, 0.0);
	paused = false;
	timeSinceLastEmission = 0.0;
	EmissionRate = emissionRate;
	SetUp();
}

ParticleSystem::~ParticleSystem()
{
}

Component* ParticleSystem::Clone()
{
	return new ParticleSystem(*this);
}

//int ParticleSystem::FindUnusedParticle()
//{
//	int particleWithLowestLifeTime = 0;
//	double lowestParticleLifeTime = 10000.0;
//	for (int i = LastUsedParticle; i < MaxParticles; i++){
//		if (ParticlesContainer[i].lifeTime < 0.0){
//			LastUsedParticle = i;
//			return i;
//		}
//		else if(ParticlesContainer[i].lifeTime < lowestParticleLifeTime)
//		{
//			particleWithLowestLifeTime = i;
//			lowestParticleLifeTime = ParticlesContainer[i].lifeTime;
//		}
//	}
//
//	for (int i = 0; i < LastUsedParticle; i++){
//		if (ParticlesContainer[i].lifeTime < 0.0){
//			LastUsedParticle = i;
//			return i;
//		}
//		else if (ParticlesContainer[i].lifeTime < lowestParticleLifeTime)
//		{
//			particleWithLowestLifeTime = i;
//			lowestParticleLifeTime = ParticlesContainer[i].lifeTime;
//		}
//	}
//
//	return particleWithLowestLifeTime;
//}

void ParticleSystem::UpdateParticles(double deltaTime)
{
	particles_data_buffer->activeCount = 0;
	int newTotalParticleCount = std::min(LastParticleIndex + NewParticles, MaxParticles-1); //improve formula for when fps is faster than emission rate
	LastParticleIndex = 0;
	DeadParticles = 0;
	ReallyAliveParticles = 0;
	int newparticles = NewParticles;
	for (int i = 0; i < newTotalParticleCount; i++){

		ParticleInfo& p = ParticlesInfo[i]; 

		if (p.lifeTime < 0.0)
		{
			if (newparticles > 0)
			{
				NewParticle(i);
				//particles_data_buffer->SetElementData(particles_data_buffer->activeCount, &ParticlesData[i]);
				//particles_data_buffer->IncreaseInstanceCount();
				newparticles--;
				LastParticleIndex = i;
				ReallyAliveParticles++;
			}
			else
			{
				p.cameraDistance = -1.0;
				DeadParticles++;
			}
		}
		else{
			UpdateParticle(i);
			//particles_data_buffer->SetElementData(particles_data_buffer->activeCount, &ParticlesData[i]);
			//particles_data_buffer->IncreaseInstanceCount();
			LastParticleIndex = i;
			ReallyAliveParticles++;
		}
	}
}

void ParticleSystem::SortParticles(){
	//std::sort(&ParticlesData[0], &ParticlesData[LastParticleIndex]);
}

//VAO
void ParticleSystem::SetUp()
{
	for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (asset.name.compare("particlePlane.particlePlane") == 0)
		{
			vao = &asset;
			particles_data_buffer = vao->dynamicVBOs[0];
			break;
		}
	}
}

//VAO
void ParticleSystem::UpdateBuffers()
{
	particles_data_buffer->Resize(ReallyAliveParticles + 1);
	particles_data_buffer->activeCount = ReallyAliveParticles + 1;
	particles_data_buffer->SetData(0, &ParticlesData[0], particles_data_buffer->layout.GetStride() * ReallyAliveParticles + 1);
	if (vao != nullptr) vao->activeCount = particles_data_buffer->activeCount;
	//particles_data_buffer->Update();
}

int ParticleSystem::Draw()
{
	if (MaxParticles > 0)
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

		if (vao != nullptr)
		{
			vao->Bind();
			vao->Draw();
		}
		

		if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_BLEND);

		return particles_data_buffer->activeCount;
	}
	return 0;
}

void ParticleSystem::SetTexture(GLuint textureID)
{
	this->TextureID = textureID;
}

void ParticleSystem::SetAdditive(bool isAdditiveMode)
{
	additive = isAdditiveMode;
}

int ParticleSystem::GetAliveParticlesCount()
{
	return vao->activeCount;
}

int ParticleSystem::GetNewParticlesCount()
{
	return NewParticles;
}

void ParticleSystem::SetMaxParticles(int maxParticles)
{
	MaxParticles = std::max(maxParticles, 0);
	LastUsedParticle = std::clamp(LastUsedParticle, 0, std::max(MaxParticles - 1, 0));
	ParticlesData.resize(MaxParticles);
	ParticlesInfo.resize(MaxParticles);
	EmissionRate = LifeTime <= 0 ? 0 : std::min(DesiredEmissionRate, (int)(MaxParticles / LifeTime));
	if (vao != nullptr && vao->vbos.size() > 0 && MaxParticles > 0)
	{
		particles_data_buffer->Resize(MaxParticles);
	}
}

void ParticleSystem::SetEmissionRate(int emissionRate)
{
	DesiredEmissionRate = emissionRate;
	EmissionRate = LifeTime <= 0 ? 0 : std::min(DesiredEmissionRate, (int)(MaxParticles / LifeTime));
}

void ParticleSystem::SetColor(const glm::vec4& color)
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
		if (MaxParticles > 0)
		{
			//calculate nr of new particles
			CalculateNewEmitedParticles(Times::Instance()->deltaTime);
			UpdateParticles(Times::Instance()->deltaTime);
			UpdateBuffers();
			//if (!additive) SortParticles();
		}
		else
		{
			vao->activeCount = 0;
		}
	}
}

inline void ParticleSystem::NewParticle(int index)
{
	ParticleData& particleData = ParticlesData[ReallyAliveParticles];
	ParticleInfo& particleInfo = ParticlesInfo[ReallyAliveParticles];
	particleInfo.lifeTime = LifeTime;
	particleData.pos = object->node->GetWorldPosition();

	// Very bad way to generate a random direction; 
	// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
	// combined with some user-controlled parameters (main direction, spread, etc)
	particleInfo.speed = Direction + glm::vec3((rand() % 2000 - 1000.0f) / 1000.0f, (rand() % 2000 - 1000.0f) / 1000.0f, (rand() % 2000 - 1000.0f) / 1000.0f) * Spread;
	particleData.color = glm::vec4((rand() % 255) / 255.0f, (rand() % 255) / 255.0f, (rand() % 255) / 255.0f, 1.0);
	particleData.size = Size;//(float)((rand() % 1000) / 2000.0f + 0.1f);
	glm::vec3 cameraToParticle = particleData.pos - CameraManager::Instance()->cameraPos;
	particleInfo.cameraDistance = glm::dot(cameraToParticle, cameraToParticle);
}

inline void ParticleSystem::UpdateParticle(int index)
{
	ParticleData& particleData = ParticlesData[index];
	ParticleInfo& particleInfo = ParticlesInfo[index];

	particleInfo.lifeTime -= Times::Instance()->deltaTime;

	// Simulate simple physics : gravity only, no collisions 
	particleInfo.speed += Force * (float)Times::Instance()->deltaTime * 0.5f;
	particleData.pos += particleInfo.speed * (float)Times::Instance()->deltaTime;
	glm::vec3 cameraToParticle = particleData.pos - CameraManager::Instance()->cameraPos;
	particleInfo.cameraDistance = glm::dot(cameraToParticle, cameraToParticle);
	if (index != ReallyAliveParticles)
	{
		ParticleData& reallyAliveParticle = ParticlesData[ReallyAliveParticles];
		ParticleInfo& reallyAliveParticleInfo = ParticlesInfo[ReallyAliveParticles];
		reallyAliveParticle = particleData;
		reallyAliveParticleInfo = particleInfo;
		particleInfo.lifeTime = -1.f;
	}
	
	//p.size -= (float)deltaTime*3.5f;
	//if (p.size < 0.f) p.size = 0.f; p.lifeTime = 0.f;
	// Fill the GPU buffer
}

inline void ParticleSystem::CalculateNewEmitedParticles(double deltaTime)
{
	//when fps is higher than emission we end up with 0 particles
	timeSinceLastEmission += deltaTime;
	if (timeSinceLastEmission > EmissionRate * 0.016)
	{
		timeSinceLastEmission = EmissionRate * 0.016;
	}
	NewParticles = timeSinceLastEmission * EmissionRate;
	if (NewParticles > 0)
	{
		timeSinceLastEmission -= (NewParticles / (double)EmissionRate);
	}
}

void ParticleSystem::SetLifeTime(float lifetime)
{
	LifeTime = lifetime;
	EmissionRate = LifeTime <= 0 ? 0 : std::min(DesiredEmissionRate, (int)(MaxParticles / LifeTime));
}

void ParticleSystem::SetDirection(const glm::vec3& direction)
{
	Direction = direction;
}

void ParticleSystem::SetSpread(float spread)
{
	Spread = spread;
}

void ParticleSystem::SetForce(const glm::vec3& force)
{
	Force = force;
}
