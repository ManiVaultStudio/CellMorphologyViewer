#pragma once

#include "MorphologyRenderer.h"

class MorphologyLineRenderer : public MorphologyRenderer
{
public:
    void init() override;
    void update() override;

    void setCellMorphology(const CellMorphology& cellMorphology) override;

private:
    mv::ShaderProgram _lineShader;
};
