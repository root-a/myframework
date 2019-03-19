#pragma once
class Object;

class Component
{
public:
	Component();
	virtual ~Component();
	virtual void Init(Object* parent);
	virtual void Update() = 0;
	Object* object;
	bool IsDynamic();
	void SetDynamic(bool newDynamic);
protected:
	bool dynamic;
};

