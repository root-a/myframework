//
// Created by marwac-9 on 9/17/15.
//
#include <GL/glew.h>
#include "ShaderManager.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

ShaderManager* ShaderManager::Instance()
{
    static ShaderManager instance;

    return &instance;
}

GLuint ShaderManager::GetCurrentShaderID()
{
	return current_shader;
	//GLint prog = 0;
	//glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
}

void ShaderManager::SetCurrentShader(GLuint id)
{
	if (current_shader != id)
	{
		current_shader = id;
		glUseProgram(id);
	}
}