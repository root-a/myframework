/*
	adding PVR loading support to stbi
*/

#ifndef HEADER_STB_IMAGE_PVR_AUGMENTATION
#define HEADER_STB_IMAGE_PVR_AUGMENTATION

/*	is it a PVR file? */
extern int      stbi__pvr_test_memory(stbi_uc const *buffer, int len);
extern int      stbi__pvr_test_callbacks(stbi_io_callbacks const *clbk, void *user);

extern TextureInfo* stbi__pvr_load_from_path(char const *filename, int req_comp);
extern TextureInfo* stbi__pvr_load_from_memory(stbi_uc const *buffer, int len, int req_comp);
extern TextureInfo* stbi__pvr_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int req_comp);

#ifndef STBI_NO_STDIO
extern int      stbi__pvr_test_filename(char const *filename);
extern int      stbi__pvr_test_file(FILE *f);
extern TextureInfo* stbi__pvr_load_from_file(FILE *f, int req_comp);
#endif

extern TextureInfo* stbi__pvr_info_from_memory(stbi_uc const *buffer, int len);
extern TextureInfo* stbi__pvr_info_from_callbacks(stbi_io_callbacks const *clbk, void *user);


#ifndef STBI_NO_STDIO
extern TextureInfo* stbi__pvr_info_from_path(char const *filename);
extern TextureInfo* stbi__pvr_info_from_file(FILE *f);
#endif

/*
//
////   end header file   /////////////////////////////////////////////////////*/
#endif /* HEADER_STB_IMAGE_PVR_AUGMENTATION */
