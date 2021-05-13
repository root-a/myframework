#include "DataInfo.h"

const char* DataInfo::PropertyTypesAsString[11] = { "INT", "UINT", "FLOAT", "CHAR", "BOOL", "MAT2", "MAT3", "MAT4", "VEC2", "VEC3", "VEC4" };
const int DataInfo::PropertyTypeSizes[11] = { sizeof(int), sizeof(unsigned int), sizeof(float), sizeof(char), sizeof(bool), sizeof(float) * 2 * 2, sizeof(float) * 3 * 3, sizeof(float) * 4 * 4, sizeof(float) * 2, sizeof(float) * 3, sizeof(float) * 4 };