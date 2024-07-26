#include "FrameBuffer.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "FBOManager.h"
#include <GL/glew.h>

FrameBuffer::FrameBuffer(GLuint existingHandle, GLenum target, int scaleX, int scaleY)
{
	handle = existingHandle;
	scaleXFactor = scaleX;
	scaleYFactor = scaleY;
}

FrameBuffer::FrameBuffer(GLenum target, int scaleX, int scaleY)
{
	glGenFramebuffers(1, &handle);
	FBOManager::Instance()->BindFrameBuffer(target, handle);
	scaleXFactor = scaleX;
	scaleYFactor = scaleY;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &handle);
}

void FrameBuffer::SpecifyTextures()
{
	for (auto& texture : textures)
	{
		if (texture->target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, texture->level);
		else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->target, texture->handle, texture->level);
	}
	for (auto& buffer : renderBuffers)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, buffer->attachment, GL_RENDERBUFFER, buffer->handle);
	}
}

void FrameBuffer::BindBuffer(GLenum target)
{
	FBOManager::Instance()->BindFrameBuffer(target, handle);
}

RenderBuffer* FrameBuffer::RegisterRenderBuffer(RenderBuffer* buffer)
{
	renderBuffers.push_back(buffer);
	return buffer;
}

void FrameBuffer::UnregisterRenderBuffer(RenderBuffer* buffer)
{
	auto res = std::find(renderBuffers.begin(), renderBuffers.end(), buffer);
	if (res != renderBuffers.end())
	{
		renderBuffers.erase(res);
	}
}

void FrameBuffer::RegisterTexture(Texture* texture)
{
	textures.push_back(texture);
	if (texture->format != GL_DEPTH_COMPONENT) attachments.push_back(texture->attachment);
}

void FrameBuffer::UnregisterTexture(Texture* texture)
{
	auto res = std::find(attachments.begin(), attachments.end(), texture->attachment);
	if (res != attachments.end())
	{
		attachments.erase(res);
	}
	auto res2 = std::find(textures.begin(), textures.end(), texture);
	if (res2 != textures.end())
	{
		textures.erase(res2);
	}
}

void FrameBuffer::SpecifyTexture(Texture * texture)
{
	if (texture->target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, texture->level);
	else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->target, texture->handle, texture->level);
}

void FrameBuffer::SpecifyRenderBuffer(RenderBuffer * rbuffer)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbuffer->attachment, GL_RENDERBUFFER, rbuffer->handle);
}

void FrameBuffer::RegisterChildBuffer(FrameBuffer* child)
{
	children.push_back(child);
}

void FrameBuffer::SpecifyTextureAndMip(Texture * texture, GLenum target, GLint level)
{
	if (target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, level);
	else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, target, texture->handle, level);
	ActivateDrawBuffers();
}

void FrameBuffer::UpdateTextures(int newBufferSizeX, int newBufferSizeY)
{
	int newXSize = newBufferSizeX * scaleXFactor;
	int newYSize = newBufferSizeY * scaleYFactor;
	if (newXSize == 0) newXSize = 1;
	if (newYSize == 0) newXSize = 1;
	for (auto& texture : textures)
	{
		texture->Update(newXSize, newYSize);
	}
	for (auto& rbuffer : renderBuffers)
	{
		rbuffer->Update(newXSize, newYSize);
	}
	for (auto& child : children)
	{
		child->UpdateTextures(newXSize, newYSize);
	}
}

void FrameBuffer::CheckAndCleanup()
{
	// Disable reading to avoid problems with older GPUs
	//can't read unless you specify it later
	//glReadBuffer(GL_NONE);
	//disable draw if we don't want to draw to all
	//glDrawBuffer(GL_NONE);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glDrawBuffer(GL_NONE);
	// Verify that the FBO is correct
	ActivateDrawBuffers();
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return;
	}

	// Restore the default framebuffer
	glBindTexture(GL_TEXTURE_2D, 0);
	FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::ReadPixelData(GLuint x, GLuint y, GLuint width, GLuint height, GLenum sendDataType, void* data, Texture* texture)
{
	BindBuffer(GL_READ_FRAMEBUFFER);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glReadBuffer(texture->attachment);
	glReadPixels(x, y, width, height, texture->format, sendDataType, data);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//UnBindBuffer(GL_READ_FRAMEBUFFER);
}

void FrameBuffer::DeleteAllTextures()
{
	for (auto& texture : textures)
	{
		delete texture;
	}
	textures.clear();
	for (auto& rbuffer : renderBuffers)
	{
		delete rbuffer;
	}
	renderBuffers.clear();
	attachments.clear();
}

void FrameBuffer::SetTextureAndMip(Texture* texture, int mip)
{
	if (texture->target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, mip);
	else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->target, texture->handle, mip);
}

void FrameBuffer::ActivateDrawBuffers()
{
	if (attachments.size() > 0)
	{
		glDrawBuffers(attachments.size(), &attachments[0]);
	}
}

void FrameBuffer::DeactivateDrawBuffers()
{
	glDrawBuffer(GL_NONE);
}