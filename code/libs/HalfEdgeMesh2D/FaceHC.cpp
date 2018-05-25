#include "FaceHC.h"

FaceHC::FaceHC()
{
	previousFace = nullptr;
	id = -1;
	visited = false;
	left.face = this;
	top.face = this;
	right.face = this;
	bottom.face = this;
}

FaceHC::~FaceHC()
{
}