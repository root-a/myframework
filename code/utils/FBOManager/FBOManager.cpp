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
	for (auto& buffer: buffers)
	{
		if (buffer->dynamicSize)
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

FrameBuffer* FBOManager::Generate2DShadowMapBuffer(int width, int height)
{
	FrameBuffer* shadowMapBuffer = GenerateFBO();
	shadowMapBuffer->dynamicSize = false;
	Texture* shadowMapTexture = shadowMapBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0));
	shadowMapTexture->SetLinear();
	shadowMapTexture->AddClampingToBorder(mwm::Vector4F(1.f, 1.f, 1.f, 1.f));
	Texture* shadowDepthTexture = shadowMapBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT));
	shadowDepthTexture->AddDefaultTextureParameters();
	shadowMapBuffer->GenerateAndAddTextures();
	shadowMapBuffer->CheckAndCleanup();
	return shadowMapBuffer;
}

FrameBuffer* FBOManager::Generate3DShadowMapBuffer(int width, int height)
{
	FrameBuffer* shadowMapBuffer = GenerateFBO();
	shadowMapBuffer->dynamicSize = false;
	Texture* shadowDepthTexture = shadowMapBuffer->RegisterTexture(new Texture(GL_TEXTURE_CUBE_MAP, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT));
	shadowDepthTexture->AddClampingToEdge();	
	shadowDepthTexture->SetNearest();	
	shadowMapBuffer->GenerateAndAddTextures();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	shadowMapBuffer->CheckAndCleanup();
	return shadowMapBuffer;

	/*
	Texture2D* tex = new Texture2D();
	// Load the texture

	GLuint cubeMapTextureID;
	glGenTextures(1, &cubeMapTextureID);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);

	for (int i = 0; i < 6; i++)
	{
		int x, y, numOfElements;
		unsigned char* data = GraphicsManager::LoadImage(textures.at(i).c_str(), &x, &y, &numOfElements, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	tex->TextureID = cubeMapTextureID;
	tex->TextureType = GL_TEXTURE_CUBE_MAP;
	return tex;
	*/
}

void FBOManager::DeleteFrameBuffer(FrameBuffer * buffer)
{
	std::vector<FrameBuffer*>::iterator iter = std::find(buffers.begin(), buffers.end(), buffer);
	if (iter != FBOManager::Instance()->buffers.end())
	{
		buffers.erase(iter);
		delete buffer;
	}
}

FrameBuffer* FBOManager::GenerateFBO()
{
	buffers.push_back(new FrameBuffer(GL_FRAMEBUFFER));
	return buffers.back();
}


