#define _CRT_SECURE_NO_DEPRECATE
#include "OBJ.h"
#include <algorithm>



bool OBJ::LoadAndIndexOBJ(const char* path)
{
	// Read our .obj file
	std::vector<Vector3F> vertices;
	std::vector<Vector2F> uvs;
	std::vector<Vector3F> normals; // Won't be used at the moment.
	bool res = LoadOBJ(path, vertices, uvs, normals);
	if (res)
	{
		std::vector<Vector3F> tangents;
		std::vector<Vector3F> bitangents;
		ComputeTangentBasis(vertices, uvs, normals, tangents, bitangents);

		indexVBO(vertices, uvs, normals, tangents, bitangents);
	}
	return res;
}

void OBJ::indexVBO(
	std::vector<Vector3F> & in_vertices,
	std::vector<Vector2F> & in_uvs,
	std::vector<Vector3F> & in_normals,
	std::vector<Vector3F> & in_tangents,
	std::vector<Vector3F> & in_bitangents
	)
{
	std::map<PackedVertex, unsigned int> VertexToOutIndex;
	
	indices.reserve(in_vertices.size());
	PackedVertex packed;
	// For each input vertex
	for (unsigned int i = 0; i<in_vertices.size(); i++){

		packed.position = in_vertices[i];
		packed.uv = in_uvs[i];
		packed.normal = in_normals[i];

		// Try to find a similar vertex in out_XXXX
		unsigned int index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !
			indices.emplace_back(index);

			// Average the tangents and the bitangents
			indexed_tangents[index] += in_tangents[i];
			indexed_bitangents[index] += in_bitangents[i];
		}
		else{ // If not, it needs to be added in the output data.
			indexed_vertices.emplace_back(in_vertices[i]);
			indexed_uvs.emplace_back(in_uvs[i]);
			indexed_normals.emplace_back(in_normals[i]);
			indexed_tangents.emplace_back(in_tangents[i]);
			indexed_bitangents.emplace_back(in_bitangents[i]);
			unsigned int newindex = (unsigned int)indexed_vertices.size() - 1;
			indices.emplace_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
	
	CalculateDimensions();
}

bool OBJ::getSimilarVertexIndex_fast(PackedVertex & packed, std::map<PackedVertex, unsigned int> & VertexToOutIndex, unsigned int & result)
{
	auto it = VertexToOutIndex.find(packed);
	if (it == VertexToOutIndex.end()){
		return false;
	}
	else{
		result = it->second;
		return true;
	}
}

bool OBJ::LoadOBJ(
	const char * path,
	std::vector<Vector3F> & out_vertices,
	std::vector<Vector2F> & out_uvs,
	std::vector<Vector3F> & out_normals
	)
{
	printf("\nLoading OBJ file %s...", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<Vector3F> temp_vertices;
	std::vector<Vector2F> temp_uvs;
	std::vector<Vector3F> temp_normals;


	FILE * file;
	file = fopen(path, "r");
	float x = 0.f, y = 0.f, z = 0.f;
	unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
	char stupidBuffer[1000];
	if (file == NULL){
		printf("\nImpossible to open the file ! Are you in the right path ?");
		//getchar();
		return false;
	}
	while (1){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0){
			//position
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			temp_vertices.emplace_back(x, y, z);
		}
		else if (strcmp(lineHeader, "vt") == 0){
			//uv
			fscanf(file, "%f %f\n", &x, &y);
			y = -y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.emplace_back(x, y);
		}
		else if (strcmp(lineHeader, "vn") == 0){
			//normal
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			temp_normals.emplace_back(x, y, z);
		}
		else if (strcmp(lineHeader, "f") == 0){
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				printf("\nFile can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.emplace_back(vertexIndex[0]);
			vertexIndices.emplace_back(vertexIndex[1]);
			vertexIndices.emplace_back(vertexIndex[2]);
			uvIndices.emplace_back(uvIndex[0]);
			uvIndices.emplace_back(uvIndex[1]);
			uvIndices.emplace_back(uvIndex[2]);
			normalIndices.emplace_back(normalIndex[0]);
			normalIndices.emplace_back(normalIndex[1]);
			normalIndices.emplace_back(normalIndex[2]);
		}
		else{
			// Probably a comment, eat up the rest of the line
			fgets(stupidBuffer, 1000, file);
		}

	}
	out_vertices.reserve(vertexIndices.size());
	out_uvs.reserve(vertexIndices.size());
	out_normals.reserve(vertexIndices.size());
	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++){

		// Get the indices of its attributes
		//unsigned int vertexIndex = vertexIndices[i];
		//unsigned int uvIndex = uvIndices[i];
		//unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		//Vector3F& vertex = temp_vertices[vertexIndex - 1];
		//Vector2F& uv = temp_uvs[uvIndex - 1];
		//Vector3F& normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.emplace_back(temp_vertices[vertexIndices[i] - 1]);
		out_uvs.emplace_back(temp_uvs[uvIndices[i] - 1]);
		out_normals.emplace_back(temp_normals[normalIndices[i] - 1]);

	}
	printf("\nFinished loading OBJ %s", path);
	fclose(file);
	return true;
}

void OBJ::ComputeTangentBasis(
	// inputs
	std::vector<Vector3F> & in_vertices,
	std::vector<Vector2F> & in_uvs,
	std::vector<Vector3F> & in_normals,
	// outputs
	std::vector<Vector3F> & out_tangents,
	std::vector<Vector3F>& out_bitangents
) {

	out_tangents.reserve(in_vertices.size());
	out_bitangents.reserve(in_vertices.size());
	for (unsigned int i = 0; i < in_vertices.size(); i += 3) {

		// Shortcuts for vertices
		Vector3F & v0 = in_vertices[i + 0];
		Vector3F & v1 = in_vertices[i + 1];
		Vector3F & v2 = in_vertices[i + 2];

		// Shortcuts for UVs
		Vector2F & uv0 = in_uvs[i + 0];
		Vector2F & uv1 = in_uvs[i + 1];
		Vector2F & uv2 = in_uvs[i + 2];

		// Edges of the triangle : postion delta
		Vector3F deltaPos1 = v1 - v0;
		Vector3F deltaPos2 = v2 - v0;

		// UV delta
		Vector2F deltaUV1 = uv1 - uv0;
		Vector2F deltaUV2 = uv2 - uv0;


		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		if (isinf(r)) printf("\nInvalid UVS");

		Vector3F tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		Vector3F bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		out_tangents.emplace_back(tangent);
		out_tangents.emplace_back(tangent);
		out_tangents.emplace_back(tangent);

		// Same thing for binormals
		out_bitangents.emplace_back(bitangent);
		out_bitangents.emplace_back(bitangent);
		out_bitangents.emplace_back(bitangent);
	}

	// See "Going Further"
	for (unsigned int i = 0; i < in_vertices.size(); i += 1)
	{
		Vector3F & n = in_normals[i];
		Vector3F & t = out_tangents[i];
		Vector3F & b = out_bitangents[i];
		// Gram-Schmidt orthogonalize
		t = (t - n * n.dotAKAscalar(t)).vectNormalize();

		// Calculate handedness
		if (n.crossProd(t).dotAKAscalar(b) > 0.0f) {
			t = t * -1.0f;
		}
	}
}

void OBJ::CalculateDimensions()
{
	Vector3F minValues;
	Vector3F maxValues;
	
	minValues = indexed_vertices[0];
	maxValues = indexed_vertices[0];
	
	for(size_t i = 0; i < indexed_vertices.size(); ++i)
	{
		Vector3F& currentVertex = indexed_vertices[i];
		maxValues.x = std::max(maxValues.x, currentVertex.x);
		minValues.x = std::min(minValues.x, currentVertex.x);
		maxValues.y = std::max(maxValues.y, currentVertex.y);
		minValues.y = std::min(minValues.y, currentVertex.y);
		maxValues.z = std::max(maxValues.z, currentVertex.z);
		minValues.z = std::min(minValues.z, currentVertex.z);
	}
	
	dimensions.x = maxValues.x - minValues.x;
	dimensions.y = maxValues.y - minValues.y;
	dimensions.z = maxValues.z - minValues.z;
	
	this->center_of_mesh = minValues.toDouble() + this->dimensions/2.0;

}

Vector3 OBJ::GetDimensions()
{
	return this->dimensions;
}

Vector3 OBJ::CenterOfOMesh()
{
	return this->center_of_mesh;
}
