#ifndef mwm_face
#define mwm_face
#include "EdgeHC.h"

class FaceHC
{
public:
	EdgeHC left, top, right, bottom;
	
	FaceHC* previousFace;
	bool visited;
	FaceHC();
	~FaceHC();
	int id;

private:
	
};

#endif
