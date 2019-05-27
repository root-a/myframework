#pragma once
#include <vector>

class Texture;

class FrameBuffer
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
	typedef int GLint;
	typedef int GLsizei;
public:
	FrameBuffer(GLenum target, int scaleX = 1, int scaleY = 1, bool dynamic = true);
	~FrameBuffer();

	GLuint handle;
	GLuint renderBufferHandle;
	GLuint renderBufferInternalFormat;
	void GenerateAndAddTextures();
	void BindBuffer(GLenum target);
	void AddRenderBuffer(GLuint internalFormat, GLsizei width, GLsizei height);
	std::vector<Texture*> textures;
	std::vector<GLenum> attachments;
	Texture* RegisterTexture(Texture* texture);
	void AttachTexture(Texture* texture);
	void AttachTexture(Texture* texture, GLenum target, GLint level = 0);
	void RegisterChildBuffer(FrameBuffer* child);
	void UpdateTextures(int newBufferSizeX, int newBufferSizeY);
	void AddDefaultTextureParameters();
	void CheckAndCleanup();
	void ReadPixelData(GLuint x, GLuint y, GLuint width, GLuint height, GLenum sendDataType, void * data, Texture* texture);
	void DeleteAllTextures();
	std::vector<FrameBuffer*> children;
	double scaleXFactor;
	double scaleYFactor;
	bool dynamicSize;
	void ActivateDrawBuffers();
	void DeactivateDrawBuffers();
private:
	FrameBuffer() {};
};