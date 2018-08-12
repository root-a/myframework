#pragma once
#include <vector>
#include <GL/glew.h>

class Texture;
class FrameBuffer;

enum FrameBufferMode
{
	read,
	draw,
	readDraw
};
class FBOManager
{
public:
	static FBOManager* Instance();
	void UpdateTextureBuffers(int windowWidth, int windowHeight);
	void BindFrameBuffer(FrameBufferMode readWrite, GLuint frameBuffer);
	void UnbindFrameBuffer(FrameBufferMode readWrite);
	FrameBuffer* Generate2DShadowMapBuffer(int width, int height);
	FrameBuffer* Generate3DShadowMapBuffer(int width, int height);
	void DeleteFrameBuffer(FrameBuffer* buffer);
	std::vector<FrameBuffer*> buffers;
	FrameBuffer* GenerateFBO();
private:
	FBOManager();
	~FBOManager();
	//copy
	FBOManager(const FBOManager&);
	//assign
	FBOManager& operator=(const FBOManager&);
};