#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "glm/vec3.hpp"      // glm vector 3, not mine
#include "glm/matrix.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <string>
#include <fstream>
#include <math.h>
#include <cmath>
#include <random>
#include <chrono>

using vec3 = glm::vec3;

///  TYPES AND STRUCTURES ///


struct ObjectData
{
    bool hit;
    glm::vec3 hitPos;
    glm::vec3 hitNormal; 
    glm::vec3 pos;
    glm::vec3 color;
};

struct Trace
{
    glm::vec3 pos;
    glm::vec3 dir;
};

struct Camera
{
    glm::vec3 pos;
    glm::vec3 dir;
    int fov;

};

// Classes

class BaseObject
{
    public:
        glm::vec3 pos;
        glm::vec3 ang;
        glm::vec3 color;

};

class Sphere : public BaseObject
{

    public:
        float radius;

        ObjectData intersect(Trace traceData)
        {
            vec3 Dir = traceData.dir;
            vec3 SpherePos = pos;
            vec3 TraceStart = traceData.pos;

            float A = 2 * glm::length(Dir) * glm::length(Dir);
            float B = 2 * glm::dot(Dir, TraceStart - SpherePos);
            float C = (glm::length(SpherePos) * glm::length(SpherePos)) + (glm::length(TraceStart) * glm::length(TraceStart)) - 2.f * glm::dot(SpherePos, TraceStart) - (radius * radius);

            float BAC4 = B * B - (2.f * A * C);

            if (BAC4 >= 0 && B < 0)
            {
                //Enter sphere
                vec3 HitPos = TraceStart + ((-sqrt(BAC4) - B) / A) * Dir;

                vec3 HitNormal = (HitPos - pos) / radius;

                return ObjectData{ true, HitPos, HitNormal, pos, color };

                //Exits sphere
                //return Start + ((sqrt(BAC4) - B) / A)*Dir
            }


            return ObjectData{ false };


        }
};

class Plane : public BaseObject
{
    public:
        ObjectData intersect(Trace traceData)
        {
            float A = glm::dot(ang, traceData.dir);

            if (A < 0)
            {
                float B = glm::dot(ang, pos - traceData.pos);

                if (B < 0)
                {
                    unsigned int x = ((traceData.pos + traceData.dir) * (B / A)).x;
                    unsigned int z = ((traceData.pos + traceData.dir) * (B / A)).z;

                    return ObjectData{ true, traceData.pos + traceData.dir * (B / A), ang, pos, vec3(((x + z) % 2) * 127 + 255) };
                    //return (Start + Dir * (B / A));

                }

            }

            return ObjectData{ false };


        }
};

///  Variable creation

std::vector<unsigned char> ImageData;
std::vector<BaseObject> ObjectArray;
unsigned int Res[2];


