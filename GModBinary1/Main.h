#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <random>
#include <chrono>

#include "types.h"
#include "objects.h"

struct Camera
{
	glm::vec3 pos;
	glm::vec3 dir;
	int fov;
};

std::vector<unsigned char> ImageData;
std::vector<std::shared_ptr<BaseObject>> ObjectArray;
unsigned int Res[2];
