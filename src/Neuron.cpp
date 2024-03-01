#include "Neuron.h"

#include "graphics/Vector3f.h"

#include <iostream>

using namespace mv;

void Neuron::center()
{
    // Find centroid and extents
    Vector3f avgPos;
    for (const auto& pos : positions)
        avgPos += pos;
    avgPos /= positions.size();

    // Center cell positions
    for (auto& pos : positions)
        pos -= avgPos;
}

void Neuron::rescale()
{
    // Find cell position ranges
    Vector3f minV(std::numeric_limits<float>::max());
    Vector3f maxV(-std::numeric_limits<float>::max());
    for (const auto& pos : positions)
    {
        if (pos.x < minV.x) minV.x = pos.x;
        if (pos.y < minV.y) minV.y = pos.y;
        if (pos.z < minV.z) minV.z = pos.z;
        if (pos.x > maxV.x) maxV.x = pos.x;
        if (pos.y > maxV.y) maxV.y = pos.y;
        if (pos.z > maxV.z) maxV.z = pos.z;
    }
    Vector3f range = (maxV - minV);
    float maxRange = std::max(std::max(range.x, range.y), range.z);
    // Rescale positions
    for (auto& pos : positions)
    {
        pos /= maxRange;
    }
    //std::cout << minV.str() << " " << maxV.str() << std::endl;
}
