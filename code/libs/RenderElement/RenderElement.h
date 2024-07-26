#pragma once
#include <unordered_map>
#include <string>

class RenderElement
{
public:
	RenderElement();
	~RenderElement();
	virtual void Execute() = 0;
	std::string name;
private:

};