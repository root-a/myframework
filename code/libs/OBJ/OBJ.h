#pragma once
#include <string.h>
#include <vector>
#include <map>
#include "MyMathLib.h"
#include <string>

class OBJ
{
public:
	std::vector<unsigned int> indices;
	std::vector<Vector3F> indexed_vertices;
	std::vector<Vector2F> indexed_uvs;
	std::vector<Vector3F> indexed_normals;
	std::vector<Vector3F> indexed_tangents;
	std::vector<Vector3F> indexed_bitangents;
	int ID;
	std::string name;
	bool LoadOBJ(
		const char * path,
		std::vector<Vector3F>& out_vertices,
		std::vector<Vector2F>& out_uvs,
		std::vector<Vector3F>& out_normals
		);
	void ComputeTangentBasis(
		std::vector<Vector3F>& in_vertices,
		std::vector<Vector2F>& in_uvs,
		std::vector<Vector3F>& in_normals,
		std::vector<Vector3F>& out_tangents,
		std::vector<Vector3F>& out_bitangents);
	void indexVBO(
		std::vector<Vector3F>& in_vertices,
		std::vector<Vector2F>& in_uvs,
		std::vector<Vector3F>& in_normals,
		std::vector<Vector3F>& in_tangents,
		std::vector<Vector3F>& in_bitangents
);
	void LoadAndIndexOBJ(const char* path);
	
	Vector3 GetDimensions();
	Vector3 CenterOfOMesh();
	
	Vector3 dimensions;
	Vector3 center_of_mesh;

	void CalculateDimensions();
private:
	struct PackedVertex{
		Vector3F position;
		Vector2F uv;
		Vector3F normal;
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

