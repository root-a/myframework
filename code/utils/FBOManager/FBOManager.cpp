#include "FBOManager.h"
#include <string>
#include "FrameBuffer.h"
#include "Vector4F.h"
#include "Texture.h"
#include <GL/glew.h>

FBOManager::FBOManager()
{
	readBuffer = 0;
	drawBuffer = 0;
}

FBOManager::~FBOManager()
{
}

FBOManager* FBOManager::Instance()
{
	static FBOManager instance;

	return &instance;
}

void FBOManager::UpdateTextureBuffers(int windowWidth, int windowHeight)
{
	for (auto& buffer: dynamicBuffers)
	{
		buffer->UpdateTextures(windowWidth, windowHeight);
	}
}

void FBOManager::BindFrameBuffer(GLuint readWriteMode, GLuint frameBuffer)
{
	switch (readWriteMode)
	{
	case GL_FRAMEBUFFER:
	{
		if (readBuffer != frameBuffer && drawBuffer != frameBuffer)
		{
			glBindFramebuffer(readWriteMode, frameBuffer);
			readBuffer = frameBuffer;
			drawBuffer = frameBuffer;
		}
		else if (readBuffer != frameBuffer)
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
			readBuffer = frameBuffer;
		}
		else if (drawBuffer != frameBuffer)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
			drawBuffer = frameBuffer;
		}
		break;
	}
	case GL_READ_FRAMEBUFFER:
	{
		if (readBuffer != frameBuffer)
		{
			glBindFramebuffer(readWriteMode, frameBuffer);
			readBuffer = frameBuffer;
		}
		break;
	}
	case GL_DRAW_FRAMEBUFFER:
	{
		if (drawBuffer != frameBuffer)
		{
			glBindFramebuffer(readWriteMode, frameBuffer);
			drawBuffer = frameBuffer;
		}
		break;
	}
	default:
		break;
	}
	//GLint drawFboId = 0, readFboId = 0;
	//glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	//glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
	//printf("current read: %d write: %d\n", readFboId, drawFboId);
	//printf("stored read: %d write: %d\n\n", readBuffer, drawBuffer);
}

FrameBuffer* FBOManager::Generate2DShadowMapBuffer(FrameBuffer* staticFrameBuffer, int width, int height)
{
	Texture* shadowMapTexture = new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
	shadowMapTexture->GenerateBindSpecify();
	shadowMapTexture->SetLinear();
	shadowMapTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));
	Texture* shadowDepthTexture = new Texture(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT);
	shadowDepthTexture->GenerateBindSpecify();
	shadowDepthTexture->SetDefaultParameters();

	staticFrameBuffer->RegisterTexture(shadowMapTexture);
	staticFrameBuffer->RegisterTexture(shadowDepthTexture);

	staticFrameBuffer->SpecifyTextures();
	
	staticFrameBuffer->CheckAndCleanup();
	return staticFrameBuffer;
}

FrameBuffer* FBOManager::Generate3DShadowMapBuffer(FrameBuffer* staticFrameBuffer, int width, int height)
{
	Texture* shadowDepthTexture = new Texture(GL_TEXTURE_CUBE_MAP, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT);
	shadowDepthTexture->GenerateBindSpecify();
	shadowDepthTexture->SetClampingToEdge();
	shadowDepthTexture->SetNearest();

	staticFrameBuffer->RegisterTexture(shadowDepthTexture);
	
	staticFrameBuffer->SpecifyTextures();
	
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	staticFrameBuffer->CheckAndCleanup();
	return staticFrameBuffer;
}

FrameBuffer* FBOManager::GetCurrentDrawFrameBuffer()
{
	for (auto fbo : dynamicBuffers)
	{
		if (fbo->handle == drawBuffer)
		{
			return fbo;
		}
	}
	for (auto fbo : staticBuffers)
	{
		if (fbo->handle == drawBuffer)
		{
			return fbo;
		}
	}
	return nullptr;
}

FrameBuffer* FBOManager::GetCurrentReadFrameBuffer()
{
	for (auto fbo : dynamicBuffers)
	{
		if (fbo->handle == readBuffer)
		{
			return fbo;
		}
	}
	for (auto fbo : staticBuffers)
	{
		if (fbo->handle == readBuffer)
		{
			return fbo;
		}
	}
	return nullptr;
}

void FBOManager::DeleteFrameBuffer(FrameBuffer * buffer)
{
	for (int i = 0; i < dynamicBuffers.size(); ++i)
	{
		if (dynamicBuffers[i] == buffer)
		{
			dynamicBuffers[i] = dynamicBuffers.back();
			dynamicBuffers.pop_back();
			delete buffer;
			return;
		}
	}
	for (int i = 0; i < staticBuffers.size(); ++i)
	{
		if (staticBuffers[i] == buffer)
		{
			staticBuffers[i] = staticBuffers.back();
			staticBuffers.pop_back();
			delete buffer;
			return;
		}
	}
}

void FBOManager::MakeStatic(FrameBuffer * buffer)
{
	for (int i = 0; i < dynamicBuffers.size(); ++i)
	{
		if (dynamicBuffers[i] == buffer)
		{
			dynamicBuffers[i] = dynamicBuffers.back();
			dynamicBuffers.pop_back();
		}
	}
	for (int i = 0; i < staticBuffers.size(); ++i)
	{
		if (staticBuffers[i] == buffer)
		{
			return;
		}
	}
	staticBuffers.push_back(buffer);
}

void FBOManager::MakeDynamic(FrameBuffer * buffer)
{
	for (int i = 0; i < staticBuffers.size(); ++i)
	{
		if (staticBuffers[i] == buffer)
		{
			staticBuffers[i] = staticBuffers.back();
			staticBuffers.pop_back();
		}
	}
	for (int i = 0; i < dynamicBuffers.size(); ++i)
	{
		if (dynamicBuffers[i] == buffer)
		{
			return;
		}
	}
	dynamicBuffers.push_back(buffer);
}

bool FBOManager::IsDynamic(FrameBuffer * buffer)
{
	for (int i = 0; i < dynamicBuffers.size(); ++i)
	{
		if (dynamicBuffers[i] == buffer)
		{
			return true;
		}
	}
	return false;
}

void FBOManager::AddFrameBuffer(FrameBuffer* buffer, bool dynamic)
{
	if (dynamic)
	{
		dynamicBuffers.push_back(buffer);
	}
	else
	{
		staticBuffers.push_back(buffer);
	}
}
