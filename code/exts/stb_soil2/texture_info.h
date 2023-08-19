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
	typedef struct PNMExtraInfo PNMExtraInfo;
	typedef struct PSDExtraInfo PSDExtraInfo;

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
		PVR,
		QOI
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
	
	struct PNMExtraInfo
	{
		int bitsPerChannel;
	};
	
	struct PSDExtraInfo
	{
		int channelCount;
		int depth;
	};

#ifdef __cplusplus
}
#endif
#endif 
