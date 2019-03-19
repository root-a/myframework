#pragma once
#include <vector>

class Texture;
class FrameBuffer;

class FBOManager
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	static FBOManager* Instance();
	void UpdateTextureBuffers(int windowWidth, int windowHeight);
	void BindFrameBuffer(GLuint readWriteMode, GLuint frameBuffer);
	FrameBuffer* Generate2DShadowMapBuffer(int width, int height);
	FrameBuffer* Generate3DShadowMapBuffer(int width, int height);
	void DeleteFrameBuffer(FrameBuffer* buffer);
	std::vector<FrameBuffer*> buffers;
	FrameBuffer* GenerateFBO();
	GLuint readBuffer;
	GLuint drawBuffer;
private:
	FBOManager();
	~FBOManager();
	//copy
	FBOManager(const FBOManager&);
	//assign
	FBOManager& operator=(const FBOManager&);
};