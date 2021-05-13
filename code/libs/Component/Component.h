#pragma once
class Object;

class Component
{
public:
	Component();
	virtual ~Component();
	virtual void Init(Object* parent);
	virtual void Update();
	Object* object;
protected:
};

