#include "Component.h"
#include "Object.h"
#include <string>
Component::Component()
{
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

Component* Component::Clone()
{
	return new Component(*this);
}
