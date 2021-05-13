#pragma once
#include <unordered_map>

class RenderElement
{
public:
	RenderElement();
	~RenderElement();
	virtual void Execute() = 0;
	std::string name;
private:

};