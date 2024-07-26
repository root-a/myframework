#include "MaterialProfile.h"

MaterialProfile::MaterialProfile()
{
	registryPtr = &registry;
}

MaterialProfile::~MaterialProfile()
{
}

void MaterialProfile::Execute()
{
	SetAndSendData();
}
