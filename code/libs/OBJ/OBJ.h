#pragma once
#include <string.h>
#include <vector>
#include <map>
#include "MyMathLib.h"
#include <string>

class OBJ
{
public:
	std::vector<unsigned char> indicesUB;
	std::vector<unsigned short> indicesUS;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;
	int ID;
	std::string name;
	std::string path;
	bool LoadOBJ(
		const char * path,
		std::vector<glm::vec3>& out_vertices,
		std::vector<glm::vec2>& out_uvs,
		std::vector<glm::vec3>& out_normals
		);
	void ComputeTangentBasis(
		std::vector<glm::vec3>& in_vertices,
		std::vector<glm::vec2>& in_uvs,
		std::vector<glm::vec3>& in_normals,
		std::vector<glm::vec3>& out_tangents,
		std::vector<glm::vec3>& out_bitangents);
	void indexVBO(
		std::vector<glm::vec3>& in_vertices,
		std::vector<glm::vec2>& in_uvs,
		std::vector<glm::vec3>& in_normals,
		std::vector<glm::vec3>& in_tangents,
		std::vector<glm::vec3>& in_bitangents
);
	bool LoadAndIndexOBJ(const char* path);
	void ProcessIndicesType();
	glm::vec3 GetDimensions();
	glm::vec3 CenterOfOMesh();
	
	glm::vec3 dimensions;
	glm::vec3 center_of_mesh;
	void* GetIndicesData();
	void CalculateDimensions();
	unsigned int indicesCount;
private:
	struct PackedVertex{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		bool operator<(const PackedVertex &that) const{
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
		};
	};
	static bool getSimilarVertexIndex_fast(
		PackedVertex & packed,
		std::map<PackedVertex, unsigned int> & VertexToOutIndex,
		unsigned int & result
		);
};

