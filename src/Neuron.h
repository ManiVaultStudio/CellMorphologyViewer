#pragma once

#include "graphics/Vector3f.h"

#include <vector>
#include <unordered_map>

class Neuron
{
public:
    void center();
    void rescale();

    std::vector<int> ids;
    std::unordered_map<int, int> idMap;
    std::vector<mv::Vector3f> positions;
    std::vector<int> types;
    std::vector<float> radii;
    std::vector<int> parents;

private:

};
