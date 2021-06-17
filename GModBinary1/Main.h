#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <random>
#include <chrono>

#include "types.h"
#include "objects.h"

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/ray.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>

struct Camera
{
	glm::vec3 pos;
	glm::vec3 dir;
	int fov;
};

std::vector<unsigned char> ImageData;
unsigned int Res[2];
