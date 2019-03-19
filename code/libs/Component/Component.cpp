#include "Component.h"
#include "Object.h"
#include <string>
Component::Component()
{
	dynamic = false;
}

Component::~Component()
{
}

void Component::Init(Object * parent)
{
	object = parent;
}

void Component::Update()
{
}

bool Component::IsDynamic()
{
	return dynamic;
}

void Component::SetDynamic(bool newDynamic)
{
	if (newDynamic != dynamic)
	{
		dynamic = newDynamic;
		object->UpdateComponentDynamicState(this);
	}
}
