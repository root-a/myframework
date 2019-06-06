#pragma once

class Shader;

class RenderPass
{
public:
	RenderPass();
	~RenderPass();
	Shader* shader;
	void AssignShader(Shader* newShader);
	void SetUp();
private:

};