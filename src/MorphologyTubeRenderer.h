#pragma once

#include "MorphologyRenderer.h"

class MorphologyTubeRenderer : public MorphologyRenderer
{
public:
    void init() override;
    void update(float t) override;

    void reloadShaders();

    void setCellMorphology(const CellMorphology& cellMorphology) override;

private:
    mv::ShaderProgram _shader;
};
