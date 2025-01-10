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
    MorphologyLineRenderer(Scene* scene) :
        MorphologyRenderer(scene),
        _cellCache(100)
    {

    }

    void init() override;
    //void update(float t) override;

    virtual void render(int index, float t) override;

private:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) override;

private:
    mv::ShaderProgram _lineShader;

    LRUCache<QString, CellRenderObject> _cellCache;
};
