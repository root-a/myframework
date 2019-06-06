#include "Shader.h"
#include <GL/glew.h>
#include <stdio.h>
#include "MyMathLib.h"
#include "Material.h"

using namespace mwm;
//this class should be used to get locations and set uniforms once before drawing, for loops get earlier locations and use default opengl glUniform functions
Shader::Shader(unsigned int ID, std::string name, ShaderPaths& sp)
{
	shaderID = ID;
	shaderName = name;
	shaderPaths = sp;
}

Shader::~Shader()
{
	glDeleteProgram(shaderID);
}

void Shader::LoadUniforms()
{
	GLint location;
	GLint count;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)
	const GLsizei bufSize = 100; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

	glGetProgramiv(shaderID, GL_ACTIVE_UNIFORMS, &count);
	printf("Active Uniforms: %d\n", count);
	for (location = 0; location < count; location++)
	{
		glGetActiveUniform(shaderID, (GLuint)location, bufSize, &length, &size, &type, name);
		uniformLocations[name] = location;

		switch (type)
		{
		case GL_FLOAT_VEC2:
			printf("vec2\n");
			break;
		case GL_FLOAT_VEC3:
			printf("vec3\n");
			break;
		case GL_FLOAT_VEC4:
			printf("vec4\n");
			break;
		case GL_BOOL:
			printf("bool\n");
			break;
		case GL_BOOL_VEC2:
			printf("bool2\n");
			break;
		case GL_BOOL_VEC3:
			printf("bool3\n");
			break;
		case GL_BOOL_VEC4:
			printf("bool4\n");
			break;
		case GL_FLOAT_MAT2:
			printf("mat2\n");
			break;
		case GL_FLOAT_MAT3:
			printf("mat3\n");
			break;
		case GL_FLOAT_MAT4:
			printf("mat4\n");
			break;
		case GL_SAMPLER_1D:
			printf("sampler1d\n");
			break;
		case GL_SAMPLER_2D:
			printf("sampler2d\n");
			break;
		case GL_SAMPLER_3D:
			printf("sampler3d\n");
			break;
		case GL_SAMPLER_CUBE:
			printf("samplerCube\n");
			break;
		case GL_INT:
			printf("int\n");
			break;
		case GL_UNSIGNED_INT:
			printf("unsigned int\n");
			break;
		case GL_FLOAT:
			printf("float\n");
			break;
		default:
			break;
		}
		//printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
		printf("Uniform loc: #%d Type: %#08x Name: %s\n", location, type, name);
	}
}

unsigned int Shader::GetLocation(const char * uniformName)
{
	if (uniformLocations.find(uniformName) == uniformLocations.end()) return -1;
	else return uniformLocations[uniformName];
}

void Shader::UpdateUniform(const char * uniformName, Matrix4F & newValue)
{
	glUniformMatrix4fv(uniformLocations[uniformName], 1, GL_FALSE, &newValue[0][0]);
}

void Shader::UpdateUniform(const char * uniformName, const Vector2F & newValue)
{
	glUniform2fv(uniformLocations[uniformName], 1, &newValue.x);
}

void Shader::UpdateUniform(const char * uniformName, const Vector3F & newValue)
{
	glUniform3fv(uniformLocations[uniformName], 1, &newValue.x);
}

void Shader::UpdateUniform(const char * uniformName, const Vector4F & newValue)
{
	glUniform4fv(uniformLocations[uniformName], 1, &newValue.x);
}

void Shader::UpdateUniform(const char * uniformName, const unsigned int newValue)
{
	glUniform1ui(uniformLocations[uniformName], newValue);
}

void Shader::UpdateUniform(const char * uniformName, const int newValue)
{
	glUniform1i(uniformLocations[uniformName], newValue);
}

void Shader::UpdateUniform(const char * uniformName, const float newValue)
{
	glUniform1f(uniformLocations[uniformName], newValue);
}

void Shader::UpdateUniform(const char * uniformName, const bool newValue)
{
	glUniform1i(uniformLocations[uniformName], newValue);
}

//GL_SHADER_TYPE 0x8B4F
//GL_FLOAT_VEC2 0x8B50
//GL_FLOAT_VEC3 0x8B51
//GL_FLOAT_VEC4 0x8B52
//GL_INT_VEC2 0x8B53
//GL_INT_VEC3 0x8B54
//GL_INT_VEC4 0x8B55
//GL_BOOL 0x8B56
//GL_BOOL_VEC2 0x8B57
//GL_BOOL_VEC3 0x8B58
//GL_BOOL_VEC4 0x8B59
//GL_FLOAT_MAT2 0x8B5A
//GL_FLOAT_MAT3 0x8B5B
//GL_FLOAT_MAT4 0x8B5C
//GL_SAMPLER_1D 0x8B5D
//GL_SAMPLER_2D 0x8B5E
//GL_SAMPLER_3D 0x8B5F
//GL_SAMPLER_CUBE 0x8B60
//#define GL_INT 0x1404
//#define GL_UNSIGNED_INT 0x1405
//#define GL_FLOAT 0x1406