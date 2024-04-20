#pragma once

#include "MorphologyRenderer.h"

class MorphologyLineSegments
{
public:
    std::vector<mv::Vector3f>   segments;
    std::vector<float>          segmentRadii;
    std::vector<int>            segmentTypes;
};

class MorphologyLineRenderer : public MorphologyRenderer
{
public:
    void init() override;
    void update(float t) override;

    void setCellMorphology(const CellMorphology& cellMorphology) override;

private:
    mv::ShaderProgram _lineShader;
};
