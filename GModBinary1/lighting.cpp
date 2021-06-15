#include "lighting.h"

using glm::vec3;


vec3 SunDirection = glm::normalize(vec3(1.f));

vec3 calculateLighting(HitResult hitResult)
{
	return (glm::dot(hitResult.normal, SunDirection) + 1) / vec3(2);
}
