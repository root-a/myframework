#pragma once
#include <unordered_map>

class Material;

namespace mwm
{
	class Matrix4F;
	class Vector2F;
	class Vector3F;
	class Vector4F;
}

struct ShaderPaths
{
	std::string vs;
	std::string fs;
	std::string gs;
};

class Shader
{
public:
	Shader(unsigned int handle, std::string name, ShaderPaths& sp);
	~Shader();
	unsigned int shaderID;
	std::string shaderName;
	ShaderPaths shaderPaths;
	std::unordered_map<std::string, unsigned int> uniformLocations;
	void LoadUniforms();
	unsigned int GetLocation(const char* uniformName);
	void UpdateUniform(const char* uniformName, mwm::Matrix4F& newValue);
	void UpdateUniform(const char* uniformName, const mwm::Vector2F& newValue);
	void UpdateUniform(const char* uniformName, const mwm::Vector3F& newValue);
	void UpdateUniform(const char* uniformName, const mwm::Vector4F& newValue);
	void UpdateUniform(const char* uniformName, const unsigned int newValue);
	void UpdateUniform(const char* uniformName, const int newValue);
	void UpdateUniform(const char* uniformName, const float newValue);
	void UpdateUniform(const char* uniformName, const bool newValue);
	std::vector<Material*> materials;
private:

};

