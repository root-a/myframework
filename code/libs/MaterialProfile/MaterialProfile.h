#pragma once
#include <vector>
#include "ObjectProfile.h"
#include "DataRegistry.h"

class MaterialProfile : public ObjectProfile
{
public:
	MaterialProfile();
	~MaterialProfile();
	DataRegistry registry;
	void Execute();
	std::string path;
private:
};