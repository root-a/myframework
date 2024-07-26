#pragma once
class Object;

class Component
{
public:
	Component();
	virtual ~Component();
	virtual void Init(Object* parent);
	virtual void Update();
	virtual Component* Clone();
	Object* object;
protected:
};

