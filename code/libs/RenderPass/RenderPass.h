#pragma once
#include <vector>
#include <unordered_set>
#include "ObjectProfile.h"
#include "Frustum.h"
#include "DataRegistry.h"

class FrameBuffer;
class Script;

class RenderPass : public ObjectProfile
{
public:
	RenderPass();
	~RenderPass();
	void SetUp();
	Frustum frustum;
	Matrix4* vp;
	void SetFrameBuffer(FrameBuffer* newFbo);
	FrameBuffer* fbo;
	void LoadLuaFile(const char * filename);
	DataRegistry registry;
	void Execute();
	Script* script;
	std::string path;
private:
};