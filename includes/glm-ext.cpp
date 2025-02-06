# include "config.hpp"
#include "glm/vec3.hpp"
bool glm::operator<(const ivec3& a,const ivec3& b)
{
	return (a.x < b.x || a.y < b.y || a.z < b.z);
}
