#ifndef mwm_halfedgemesh2dsquaredhc
#define mwm_halfedgemesh2dsquaredhc

class FaceHC;
//Implementation of half edge mesh for 2D, it generates grid based mesh where cells are made of squares, super fast with memory optimizations

class HalfEdgeMesh2DSquaredHC
{
public:
	HalfEdgeMesh2DSquaredHC();
	~HalfEdgeMesh2DSquaredHC();
	void Construct(const unsigned char* pMap, const int nMapWidth, const int nMapHeight);

	FaceHC* faces;

private:
	
	
};

#endif
