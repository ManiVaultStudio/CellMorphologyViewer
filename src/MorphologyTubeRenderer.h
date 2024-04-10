#pragma once

#include "MorphologyRenderer.h"

class MorphologyTubeRenderer : public MorphologyRenderer
{
public:
    void init() override;
    void update() override;

    void setCellMorphology(const CellMorphology& cellMorphology) override;

private:
    mv::ShaderProgram _shader;

    float t = 0;
};
