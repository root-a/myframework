#include "MathUtils.h"

namespace mwm
{
MathUtils::MathUtils()
{
}

MathUtils::~MathUtils()
{
}
double
MathUtils::ToRadians(double degrees)
{
	return (degrees * PI) / 180.0;
}
double
MathUtils::ToDegrees(double radians)
{
	return (radians * 180.0) / PI;
}
double MathUtils::PI = 3.14159265359;
}