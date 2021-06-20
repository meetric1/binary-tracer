#pragma once

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/ray.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <string>

#include "glm/glm.hpp"
#include "GarrysMod/Lua/Interface.h"

void getAllMeshes(GarrysMod::Lua::ILuaBase* LuaBase);

void printLua(GarrysMod::Lua::ILuaBase* LUA, std::string text);