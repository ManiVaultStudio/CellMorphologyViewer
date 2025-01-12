#pragma once

#include "Scene.h"

#include "LRUCache.h"

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"

#include <QOpenGLFunctions_3_3_Core>

#include <QMatrix4x4>
#include <QString>

class CellMorphology;

class CellRenderObject
{
public:
    GLuint vao = 0; // Vertex array object
    GLuint vbo = 0; // Vertex buffer object
    GLuint rbo = 0; // Radius buffer object
    GLuint tbo = 0; // Type buffer object

    int numVertices = 0;

    mv::Vector3f ranges;
    float maxExtent;
    mv::Vector3f centroid;
};

class MorphologyRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
    MorphologyRenderer(Scene* scene) :
        _scene(scene),
        _aspectRatio(1)
    {

    }

    virtual void init() = 0;
    void resize(int w, int h);
    void update(float t);

    virtual void render(int index, float t) = 0;

    void buildRenderObjects();

    int getNumRenderObjects() { return _cellRenderObjects.size(); }

protected:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) = 0;

protected:
    Scene* _scene;

    CellRenderObject _morphologyView;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;

    float _aspectRatio;

    std::vector<CellRenderObject> _cellRenderObjects;
};
