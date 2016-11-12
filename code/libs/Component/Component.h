#pragma once
class Object;

class Component
{
public:
	Component();
	virtual ~Component();
	Object* object;
	virtual void Update() = 0;
private:

};

