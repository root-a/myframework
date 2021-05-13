#ifndef HEADER_TEXTURE_INFO
#define HEADER_TEXTURE_INFO
#ifdef __cplusplus
extern "C" {
#endif

	typedef enum TextureTypes TextureTypes;
	typedef struct TextureInfo TextureInfo;
	typedef struct DDSExtraInfo DDSExtraInfo;
	typedef struct GIFExtraInfo GIFExtraInfo;
	typedef struct PVRExtraInfo PVRExtraInfo;

	enum TextureTypes
	{
		BMP,
		PNG,
		JPG,
		TGA,
		DDS,
		HDR,
		PSD,
		GIF,
		PIC,
		PNM,
		PKM,
		PVR
	};

	struct TextureInfo
	{
		enum TextureTypes type;
		int width;
		int height;
		int numOfElements;
		void* data;
		void* extraInfo;
	};

	struct DDSExtraInfo
	{
		unsigned int nrOfMips;
		unsigned int fourCC;
		int nrOfCubeMapFaces;
		int compressed;
	};

	struct GIFExtraInfo
	{
		int z;
	};

	struct PVRExtraInfo
	{
		int compressed;
	};

#ifdef __cplusplus
}
#endif
#endif 
