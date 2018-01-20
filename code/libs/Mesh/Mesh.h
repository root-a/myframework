#pragma once
class OBJ;

class Mesh
{
public:
	Mesh();
	~Mesh();
	unsigned int vaoHandle;
	unsigned int vertexbuffer;
	unsigned int uvbuffer;
	unsigned int normalbuffer;
	unsigned int elementbuffer;
	unsigned int indicesSize;
	OBJ* obj;
private:

};

