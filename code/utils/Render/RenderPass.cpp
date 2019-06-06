#include "RenderPass.h"
#include "Shader.h"

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::AssignShader(Shader * newShader)
{
	shader = newShader;
}

void RenderPass::SetUp()
{
}
