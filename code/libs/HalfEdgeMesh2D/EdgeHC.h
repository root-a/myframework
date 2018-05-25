#ifndef mwm_edge
#define mwm_edge

class FaceHC;

class EdgeHC
{
public:
	EdgeHC *pair;
	FaceHC *face;

	EdgeHC();
	~EdgeHC();

private:

};

#endif
