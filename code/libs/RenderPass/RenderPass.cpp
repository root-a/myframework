#include "RenderPass.h"
#include "FrameBuffer.h"
#include "FBOManager.h"
#include <GL/glew.h>
#include "Script.h"

RenderPass::RenderPass()
{
	fbo = nullptr;
	script = new Script();
	registryPtr = &registry;
}

RenderPass::~RenderPass()
{
}

void RenderPass::SetUp()
{
	fbo != nullptr ? fbo->BindBuffer(GL_FRAMEBUFFER) : FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, 0);
	script->Call();
}

void RenderPass::SetFrameBuffer(FrameBuffer * newFbo)
{
	fbo = newFbo;
}

void RenderPass::LoadLuaFile(const char * filename)
{
	std::string directorywithfilename = "resources\\passes\\";
	directorywithfilename.append(filename);
	directorywithfilename.append(".lua");
	script->LoadLuaFile(directorywithfilename.c_str());
}

void RenderPass::Execute()
{
	SetAndSendData();
	SetUp();
}